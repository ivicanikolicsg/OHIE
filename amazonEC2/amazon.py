
from __future__ import print_function
import argparse
import subprocess
import datetime
import os
import subprocess
from subprocess import PIPE
import json
from params import *
from instances import *
import random
import sys

if sys.version[0]=="2": input=raw_input



def get_instances_from_json( json_txt ):
    instances = []
    if 'Reservations' in json_txt:
        for r in json_txt['Reservations']:  
            if 'Instances' in r:
                for i in r['Instances']:
                    instances.append( i )

    return instances


def query_instances_by_regions( full_regions ):
    qs = []
    for reg in Regions:
        if reg["RegionName"] in full_regions:

            qs.append( (reg["RegionName"], 
                subprocess.Popen( 'aws ec2 describe-instances --region '+reg["RegionName"] ,  shell=True, stdin=PIPE,stdout=PIPE) ) )


    instances = []
    for q in qs:

        rd = q[1].communicate()[0]
        rd = json.loads(rd)
        single_instances = get_instances_from_json( rd )
        for s in single_instances:
            s['myregion'] = q[0]
            s['mykey'] = ('keys/'+full_regions[ s['myregion'] ]['key']) if s['myregion'] in full_regions else ''
            s['mycity'] = full_regions[ s['myregion'] ]['city'] if s['myregion'] in full_regions else ''
        instances.extend( single_instances)
            
    return instances


def query_regions_for_max_instance( full_regions ):
    qs = []
    for r in full_regions :
        qs.append( (r, 
            subprocess.Popen( 'aws ec2 describe-account-attributes --attribute-names "max-instances"  --region '+r ,  shell=True, stdin=PIPE,stdout=PIPE) ) )


    for q in qs:

        rd = q[1].communicate()[0]
        rd = json.loads(rd)

        if 'AccountAttributes' in rd:
            for r in rd['AccountAttributes']:  
                if 'AttributeName' in r and r['AttributeName'] == 'max-instances' and 'AttributeValues' in r:
                    full_regions[q[0]]['max_instances'] = r['AttributeValues'][0]['AttributeValue']

            
    return full_regions


def run_new_instances( instances, n ):

    print('='*80+'\n'+ 'Running %d new instances: ' % n)

    # first count how many instances are running
    count = 0
    for i in instances:
        if is_running_instance(i): count += 1
    if count + n > max_instances:
        print("\033[31;1m[-] Cannot run any more instances.\033[0m"  )
        print('\tYou have %d running instance(s) and want to run %d additional.\n\tThe max number of instances cannot be more than %d (see max_instances in params.py.\n\tExiting...' % (count, n, max_instances))
        exit()

    qs = []
    for i in range(n):

        # find region with min count
        mink = ''
        for k in full_regions:
            if '' == mink: mink = k
            if full_regions[k]['count'] < full_regions[mink]['count']: mink = k

        # run the instance
        print('\033[32;1m[+] Running instance in %s with region %s\033[0m' % (full_regions[mink]['city'], mink) )
        
        s = 'aws ec2 run-instances --instance-type '+instance_type+' --launch-template  LaunchTemplateId='+full_regions[mink]['lt']+' --region '+mink
        qs.append(  subprocess.Popen( s,  shell=True ) ) 

        # increase the counter
        full_regions[mink]['count'] += 1


    for q in qs:
        q.wait()

    print('\033[32;1m\n[+] Done running new instances \033[0m' )




def terminate_all_instances( instances):

    print('='*80+'\n'+ 'Terminate all instances ' )
    
    qs = []
    for i in instances:

        print('\033[32;1m[+] Terminating instance in %s with region %s\033[0m' % ( get_instance_city(i), get_instance_region(i)) )
        s = 'aws ec2 terminate-instances --instance-ids '+i['InstanceId'] + ' --region ' + get_instance_region(i)
        qs.append(  subprocess.Popen( s,  shell=True ) ) 


    for q in qs:
        q.wait()

    print('\033[32;1m\n[+] Done terminating instances \033[0m' )




def bootstrap_all_instances( instances, over=True ):


    # create the archive file
    os.system('cd '+FOLDER_NODE_CPP+';tar -czvf node.tar.gz *.c* *.h* _ecc_key Makefile;')

    kl = []
    for i in instances:

        if is_running_instance( i ):

            print('='*80+'\n'+ 'Bootstrapping: '+get_instance_city(i)  +' region '+get_instance_region(i) + " ip  "+get_instance_ip(i) )

            ip = get_instance_ip(i)
            key_pair_file = get_instance_key(i)

            # Copy the folder    
            cmd_cf = 'scp -i '+key_pair_file+' -r '+FOLDER_NODE_CPP+'/node.tar.gz '+username+'@'+ip+':~/'
            
            # Install libraries && compile
            inlib = ''
            if not over:
                inlib += 'if [ ! -f '+FOLDER_NODE_CPP+'/Nodeaws ];then '
            inlib += 'sudo apt-get update; sudo apt install -y g++ make libboost-dev libssl1.0-dev libboost-system-dev libboost-thread-dev libboost-chrono1.65-dev libboost-filesystem-dev'
            inlib += ';mkdir -p '+FOLDER_NODE_CPP
            inlib += ';cd ~/'+FOLDER_NODE_CPP
            inlib += ';tar -xf ../node.tar.gz -C .'
            inlib += ';fuser -k out*'
            inlib += ';make aws'
            if not over:
                inlib += '; fi';

            cmd_ilc = 'ssh -i "'+key_pair_file+'"  '+username+'@'+ip+' "'+inlib+'"'


            kl.append( subprocess.Popen( cmd_cf+";"+cmd_ilc , 
                shell=True,stdin=None, stdout=None, stderr=None, close_fds=True))

    for l in kl:
        l.wait()

    print('\033[32;1m\n[+] Done boostrapping nodes \033[0m' )







def check_bootstraped( instances ):

    print('='*80+'\n'+ 'Check on bootstrapped instances: ')

    countall = 0
    countboot= 0
    qs = []
    for i in instances:

        if is_running_instance( i ):

            countall += 1

            ip = get_instance_ip(i)
            key_pair_file = get_instance_key(i)

            s = 'ssh -i "'+key_pair_file+'"  '+username+'@'+ip+' \'if [ -f ~/'+FOLDER_NODE_CPP+'/Nodeaws ]; then echo yes; else echo no; fi\''
            qs.append( (i, subprocess.Popen( s ,  shell=True, stdin=PIPE,stdout=PIPE) ) )



    for q in qs:

        rd = q[1].communicate()[0].decode('ascii')
        if rd.find('yes')>=0:
            print('\033[32;1m[+] Instance %s %s  bootstrapped\033[0m' % (get_instance_city(q[0]), get_instance_ip(q[0])) )
            countboot += 1
        else:
            print("\033[31;1m[-] Instance %s %s NOT bootstrapped\033[0m" % (get_instance_city(q[0]), get_instance_ip(q[0]))  )

    print('Bootstrapped: %d  / %d ' % (countboot, countall ))




def kill_launched_nodes( instances ):

    print('='*80+'\n'+ 'Killing running nodes on instances: ')
    
    kl = []
    for i in instances: 
        if is_running_instance( i ):
        
            print('\t[ ] %s with %s ' % (get_instance_city(i), get_instance_ip(i)) )

            ip = get_instance_ip( i )
            k = 'cd '+FOLDER_NODE_CPP+'; fuser -k out*;'

            kl.append( subprocess.Popen( 'ssh -i "'+ get_instance_key(i) +'"  '+username+'@'+ip+' "'+k+'"' , 
                shell=True,stdin=None, stdout=None, stderr=None, close_fds=True))

    for l in kl:
        l.wait()

    print('\033[32;1m\n[+] Done killing nodes \033[0m' )


def get_running_nodes( instances ):

    print('='*80+'\n'+ 'Getting number of running NODES: ')
    no = 0
    qs = []
    for i in instances: 
        if is_running_instance( i ):
            ip = get_instance_ip( i )
            ps = ' ps aux|grep Nodea|grep -v aux|grep aws|wc -l'
            full = 'ssh -i "'+ get_instance_key(i)+'"  '+username+'@'+ip+' \''+ps+'\''

            qs.append( (i, subprocess.Popen( full ,  shell=True, stdin=PIPE,stdout=PIPE) ) )
 

    for q in qs:

        rd = q[1].communicate()[0]
        rd = rd.decode('ascii').replace("\n","")
        try: 
            int(rd)
            no += int(rd)
        except ValueError:
            pass
        
        print('Instance %15s %15s : %s' % ( get_instance_city(q[0]), get_instance_ip(q[0]), rd ) )

    print('Total: %d' % no)
    
    return no




def print_hashes(instances, only_one=-1):

    print('='*80+'\n'+ 'Printing hashes ')

    kl = []
    for i in instances: 
        if is_running_instance( i ):
            if only_one>=0:
                c1 = " \"cd node_cpp_code/_Hashes;find . -name '*' -type f -exec grep '^"+str(only_one)+":' {} \\;\""
            else:
                c1 = " \"cd node_cpp_code/_Hashes;find . -name '*' -type f -exec tail -n 1 {} \\;\""
            c1 = "ssh -i "+get_instance_key(i)+" "+username+"@"+get_instance_ip( i )+ c1 
            kl.append( subprocess.Popen( c1 ,  shell=True, stdin=PIPE,stdout=PIPE))


    d = {}
    count = 0
    for l in kl:
        rd =  l.communicate()[0].decode('ascii')
        ls = rd.split('\n')
        for ln in ls:
            v = ln.split(':')
            if len(v) == 2:
                if ln in d: d[ln] += 1
                else: d[ln] = 1;
                count += 1

    for ln in d:
        print('\t%20s ::: %5d  :::    %4.0f %%' % (ln, d[ln], 100.0*d[ln]/count ))

    print('\033[32;1m\n[+] Done relaunching nodes \033[0m' )



def get_dlat(instances):

    print('='*80+'\n'+ 'Getting diameter and latency ')

    kl = []
    for i in instances: 
        if is_running_instance( i ):
            c1 = " cd node_cpp_code; grep '^0' _Pings/*8* | awk -v sp=':' 'BEGIN{max0=0}{ if (\\$4>0+max0) max0=\\$4 fi;} END{printf max0 sp};';grep '^1' _Pings/* | awk 'BEGIN{max1=0}{if (\\$3>0+max1) max1=\\$3 fi;} END{printf max1};'"
            c1 = "ssh -i "+get_instance_key(i)+" "+username+"@"+get_instance_ip( i )+ ' "' + c1 + '"'
            kl.append( subprocess.Popen( c1 ,  shell=True, stdin=PIPE,stdout=PIPE))

    max_diam = 0
    max_late = 0
    for l in kl:
        rd =  l.communicate()[0].decode('ascii')
        ls = rd.split('\n')
        for ln in ls:
            v = ln.split(':')
            if len(v) == 2:
                print(ln)
                if int(v[0]) > max_late: max_late = int( v[0] )
                if int(v[1]) > max_diam: max_diam = int( v[1] )

    print("Max diameter: %d   max latency:  %d" % (max_diam, max_late ))

    print('\033[32;1m\n[+] Done \033[0m' )


def check_pattern(instances, pattern):

    print('='*80+'\n'+ 'Check pattern :%s' % pattern)

    kl = []
    for i in instances: 
        if is_running_instance( i ):
            c1 = " 'cd node_cpp_code; for filename in ./out*; do grep \""+pattern+"\" $filename|tail -1; done'"
            c1 = "ssh -i "+get_instance_key(i)+" "+username+"@"+get_instance_ip( i ) + c1 
            kl.append( subprocess.Popen( c1 , shell=True,stdin=None, stdout=None, stderr=None, close_fds=True))


    for l in kl:
        l.wait()

    print('\033[32;1m\n[+] Done checking pattern \033[0m' )




def relaunch_nodes(instances, impose_limit=True, rate_Mbps="40"):

    print('='*80+'\n'+ 'Relaunching ::: throughput_limit %d ::: rate %s Mbps' % (impose_limit, rate_Mbps))


    # create the necessary files, i.e. _peers_ip, _configuration, _network
    create_cpp_network_config_files(instances, FILE_CONFIG_CPP)

    # copy the files
    kl = []
    for i in instances: 
        if is_running_instance( i ):

            ip = get_instance_ip( i )
            ip_private = get_instance_ip_private(i)
            ikey = get_instance_key(i);

            print('\t[ ] '+get_instance_city(i)+' ' + ip)

            # Copy the config files
            c1 = 'scp -i "'+ikey+'" '+FILE_CONFIG_CPP+' ' + FILE_NETWORK_CPP + ' ' + FILE_NETWORK_PEERS_CPP + ' '+username+'@'+ip+':~/'+FOLDER_NODE_CPP+'/'

            # Kill && start the nodes
            c1 += ";ssh -i "+ikey+" "+username+"@"+ip+" "
            c1 += "'"

            if impose_limit:
                c1 += 'rdev="lo";for iface in $(ifconfig | cut -d " " -f1| tr ":" "\n" | awk NF); do if [ "$iface" != "lo" ]; then rdev=$iface; fi;  done; echo "Network interface:"$rdev":";'
                c1 += "sudo tc qdisc del dev $rdev root;"
                c1 += "sudo tc qdisc add dev $rdev root handle 1: htb;"
                c1 += "sudo tc class add dev $rdev parent 1: classid 1:1 htb rate 1000Mbps;"
                c1 += "for((i=2;i<="+str(nodes+1)+";i++)) do "
                c1 += "     sudo tc class add dev $rdev parent 1:1 classid 1:$i htb rate 1000Mbps;"
                c1 += "     sudo tc qdisc add dev $rdev handle $i: parent 1:$i tbf rate "+rate_Mbps+"Mbit burst 500k latency 10000s;"
                c1 += "     sudo tc filter add dev $rdev pref $i protocol ip u32 match ip dport $(("+str(start_port)+"+$i-2)) 0xffff flowid 1:$i;"
                c1 += "done;"

            c1 += "cd node_cpp_code; fuser -k out*; rm out*; rm -rf _Blockchains/*;rm -rf _Sessions/*;rm -rf _Hashes/*;rm -rf _Pings/*;rm -rf _Blocks/*;for((i="+str(start_port)+";i<"+str(start_port+nodes)+";i++))\
                do echo -n $i+" ";nohup ./Nodeaws $i _network "+ip+" "+ip_private+" >> out$i.txt & done"
            c1 += "'"

            kl.append( subprocess.Popen( c1 , 
                shell=True,stdin=None, stdout=None, stderr=None, close_fds=True))


    for l in kl:
        l.wait()

    print('\033[32;1m\n[+] Done relaunching nodes \033[0m' )



def special_relaunch_nodes(instances, impose_limit=True, rate_Mbps="40"):

    print('='*80+'\n'+ 'SPECIAL Relaunching ::: throughput_limit %d ::: rate %s Mbps' % (impose_limit, rate_Mbps))

    create_cpp_network_config_files(instances, FILE_CONFIG_CPP)

    with open(FILE_CONFIG_CPP,'a+') as f:
        f.write("STORE_BLOCKS=1\n")
        f.write("BLOCKS_STORE_FREQUENCY = 1\n")
        f.write("PING_MIN_WAIT=2\n")
        f.write("PING_MAX_WAIT=4\n")
        f.write("PING_REPEAT=6\n")
        f.write("EXPECTED_MINE_TIME_IN_MILLISECONDS=10000000000\n")
        f.write("MAX_MINE_BLOCKS=0\n")

        f.close()


    # randomely select special instance
    count = 0
    for i in instances:
        if is_running_instance( i ):
            count += 1
    special_node = random.randint(0,count-1)
    special_port = random.randint(start_port, start_port + nodes)



    # copy and run the files
    kl = []
    count = 0
    for i in instances: 
        if is_running_instance( i ):

            ip = get_instance_ip( i )
            ip_private = get_instance_ip_private(i)
            ikey = get_instance_key(i);

            print('\t[ ] '+get_instance_city(i)+' ' + ip)

            # Copy the config files
            c1 = 'scp -i "'+ikey+'" '+FILE_CONFIG_CPP+' ' + FILE_NETWORK_CPP + ' ' + FILE_NETWORK_PEERS_CPP + ' '+username+'@'+ip+':~/'+FOLDER_NODE_CPP+'/'

            # Kill && start the nodes
            c1 += ";ssh -i "+ikey+" "+username+"@"+ip+" "
            c1 += "'"

            if impose_limit:
                c1 += 'rdev="lo";for iface in $(ifconfig | cut -d " " -f1| tr ":" "\n" | awk NF); do if [ "$iface" != "lo" ]; then rdev=$iface; fi;  done; echo "Network interface:"$rdev":";'
                c1 += "sudo tc qdisc del dev $rdev root;"
                #c1 += "sudo tc filter del dev $rdev;"
                c1 += "sudo tc qdisc add dev $rdev root handle 1: htb;"
                c1 += "sudo tc class add dev $rdev parent 1: classid 1:1 htb rate 1000Mbps;"
                c1 += "for((i=2;i<="+str(nodes+1)+";i++)) do "
                #c1 += "echo $i;"
                c1 += "     sudo tc class add dev $rdev parent 1:1 classid 1:$i htb rate 1000Mbps;"
                c1 += "     sudo tc qdisc add dev $rdev handle $i: parent 1:$i tbf rate "+str(rate_Mbps)+"Mbit burst 500k latency 10000s;"
                #c1 += "     sudo tc filter add dev $rdev pref $i protocol ip u32 match ip sport $(("+str(start_port)+"+$i-2)) 0xffff flowid 1:$i;"
                c1 += "     sudo tc filter add dev $rdev pref $i protocol ip u32 match ip dport $(("+str(start_port)+"+$i-2)) 0xffff flowid 1:$i;"
                c1 += "done;"

            c1 += "cd node_cpp_code; fuser -k out*; rm out*; rm -rf _Blockchains/*;rm -rf _Sessions/*;rm -rf _Hashes/*;rm -rf _Pings/*;rm -rf _Blocks/*;"
            if count != special_node:
                c1 += "for((i="+str(start_port)+";i<"+str(start_port+nodes)+";i++))\
                    do echo -n $i+" ";nohup ./Nodeaws $i _network "+ip+" "+ip_private+" >> out$i.txt & done"
            else:
                c1 += "for((i="+str(start_port)+";i<"+str(special_port)+";i++))\
                    do echo -n $i+" ";nohup ./Nodeaws $i _network "+ip+" "+ip_private+" >> out$i.txt & done"
            
                c1 += ";for((i="+str(special_port+1)+";i<"+str(start_port+nodes)+";i++))\
                    do echo -n $i+" ";nohup ./Nodeaws $i _network "+ip+" "+ip_private+" >> out$i.txt & done"
            
            count += 1
            
            c1 += "'"

            kl.append( subprocess.Popen( c1 , 
                shell=True,stdin=None, stdout=None, stderr=None, close_fds=True))


    for l in kl:
        l.wait()


    # create modified config file
    with open( FILE_CONFIG_CPP,'a+') as f:
        f.write("EXPECTED_MINE_TIME_IN_MILLISECONDS=45000\n")
        f.write("MAX_MINE_BLOCKS=1\n")
        f.close()

    kl = []
    count = 0
    for i in instances: 
        if is_running_instance( i ):
            if count == special_node:

                ip = get_instance_ip( i )
                ip_private = get_instance_ip_private(i)
                ikey = get_instance_key(i);

                print('\n\t[ ] Starting special in '+get_instance_city(i)+' ' + ip)

                # Copy the config files
                c1 = 'scp -i "'+ikey+'" '+FILE_CONFIG_CPP+' ' + FILE_NETWORK_CPP + ' ' + FILE_NETWORK_PEERS_CPP + ' '+username+'@'+ip+':~/'+FOLDER_NODE_CPP+'/'
                c1 += ";ssh -i "+ikey+" "+username+"@"+ip+" "
                c1 += "'"
                c1 += "cd node_cpp_code;"
                c1 += "nohup ./Nodeaws "+str(special_port)+" _network "+ip+" "+ip_private+" >> out"+str(special_port)+".txt &"
                c1 += "'"
                kl.append( subprocess.Popen( c1 , shell=True,stdin=None, stdout=None, stderr=None, close_fds=True))
            
            count += 1


    for l in kl:
        l.wait()


    print('\033[32;1m\n[+] Done special relaunching nodes \033[0m' )


def dump_data(instances):

    print('='*80+'\n'+ 'Dumping data ')

    folder = "Dump/"+datetime.datetime.now().strftime("%Y-%m-%d-%H:%M:%S")

    # Create folder
    os.system('mkdir -p Dump')
    os.system('mkdir -p '+folder)

    kl = []
    for i in instances: 
        if is_running_instance( i ):

            full_folder = folder+'/'+get_instance_ip(i)
            os.system('mkdir -p '+ full_folder)


            c1 = 'ssh -i "'+get_instance_key(i)+'"  '+username+'@'+get_instance_ip(i)
            c1 +=' \'cd ~/'+FOLDER_NODE_CPP+'; tar czf '+get_instance_ip(i)+'.tar.gz out* '
            c1 +='_Hashes '
            c1 +='_Pings '
            c1 +='_Blocks '
            c1 +='\'' 
            c1 +='; scp -i "'+get_instance_key(i)+'" -r '+username+'@'+get_instance_ip(i)+':~/'+FOLDER_NODE_CPP+'/'+get_instance_ip(i)+'.tar.gz '+ full_folder 
            kl.append( subprocess.Popen( c1 ,  shell=True,stdin=None, stdout=None, stderr=None, close_fds=True))


    for l in kl:
        l.wait()

    print('\033[32;1m\n[+] Done dumping data \033[0m' )


def dump_blocks(instances):

    print('='*80+'\n'+ 'Dumping blocks ')

    folder = "Dump/"+datetime.datetime.now().strftime("%Y-%m-%d-%H-%M-%S")

    # Create folder
    os.system('mkdir -p Dump')
    os.system('mkdir -p '+folder)

    kl = []
    for i in instances: 
        if is_running_instance( i ):

            full_folder = folder+'/'+get_instance_ip(i)
            os.system('mkdir -p '+ full_folder)

            c1 = ' scp -i "'+get_instance_key(i)+'" -r '+username+'@'+get_instance_ip(i)+':~/'+FOLDER_NODE_CPP+'/_Blocks '+full_folder+"/"

            kl.append( subprocess.Popen( c1 ,  shell=True,stdin=None, stdout=None, stderr=None, close_fds=True))


    for l in kl:
        l.wait()

    print('\033[32;1m\n[+] Done dumping blocks \033[0m' )


def dump_pings(instances):

    print('='*80+'\n'+ 'Dumping pings ')

    folder = "Dump/"+datetime.datetime.now().strftime("%Y-%m-%d-%H-%M-%S")

    # Create folder
    os.system('mkdir -p Dump')
    os.system('mkdir -p '+folder)

    kl = []
    for i in instances: 
        if is_running_instance( i ):

            full_folder = folder+'/'+get_instance_ip(i)
            os.system('mkdir -p '+ full_folder)
            c1 = ' scp -i "'+get_instance_key(i)+'" -r '+username+'@'+get_instance_ip(i)+':~/'+FOLDER_NODE_CPP+'/_Pings '+full_folder+"/"
            kl.append( subprocess.Popen( c1 ,  shell=True,stdin=None, stdout=None, stderr=None, close_fds=True))

    for l in kl:
        l.wait()

    print('\033[32;1m\n[+] Done dumping pings \033[0m' )


def list_instances(instances):

    print("*"*80)
    
    dic = {}
    for i in instances:
        st = ''
        if 'State' in i and 'Name' in i['State']: 
            st = i['State']['Name']
        print('[+] Instance %15s with %14s is %s ' % (get_instance_city(i), get_instance_ip(i), st ) )

        if st in dic: dic[st] += 1
        else: dic[st] = 1

    print("Total:")
    for d in dic:
        print('\t%s : %d' % (d,dic[d]))

    print("*"*80)



# get all instances with aws
instances = query_instances_by_regions( full_regions )
#exit()

print('='*80+'\n'+ 'List of instances : ')
for i in instances:
    if is_running_instance( i ):

        print('[+] Instance %15s ::: %14s ::: %15s :::  ssh -i %s ubuntu@%s' % (get_instance_city(i), get_instance_region(i),  get_instance_ip(i), get_instance_key(i), get_instance_ip(i)) )
        
        # Count active instances
        if 'count' in full_regions[ i['myregion'] ]:
            full_regions[ i['myregion'] ]['count'] += 1
        else:
            full_regions[ i['myregion'] ]['count'] = 1
cnt = 0
for f in full_regions:
    print('%15s : %d ' % (full_regions[f]['city'], full_regions[f]['count']) )
    cnt += full_regions[f]['count']
print('-'*20+'\n' + "Total: %d" % cnt + '\n' + '-'*20 + '\n\n')
        

impose_limit = True
rate_Mbps="40"
random.seed(datetime.datetime.now())

parser = argparse.ArgumentParser()
parser.add_argument("--list",        help="Just list all instances", action='store_true')
parser.add_argument("--getmax",        help="Get the maximum instances that can be rented per region", action='store_true')
parser.add_argument("--boot",        help="Boostrap", action='store_true')
parser.add_argument("--bootover",        help="Forced bootstrap, i.e. overwrite", action='store_true')
parser.add_argument("--checkboot",        help="Check which nodes have been bootstrapped", action='store_true')
parser.add_argument("--compare",        help="Print the last hashes of the chain 0 in all nodes", action='store_true')
parser.add_argument("--compareon",        type=str,   help="Print the i-th hash of chain 0", action='store', nargs=1)
parser.add_argument("--pattern",        type=str,   help="Grep on pattern in all out files", action='store', nargs=1)
parser.add_argument("--dlat",        help="Get diameter and latency", action='store_true')


parser.add_argument("--launch",        help="Launch the network, i.e. restart all running nodes ", action='store_true')
parser.add_argument("--speciallaunch",        help="Launch the network, i.e. restart all running nodes ", action='store_true')
parser.add_argument("--killnodes",        help="Kill all launched instances", action='store_true')
parser.add_argument("--nodes",        help="Get number of running nodes on all instances", action='store_true')
parser.add_argument("--dump",        help="Transfer logs,etc to local machine", action='store_true')
parser.add_argument("--dumpblocks",        help="Transfer only blocks to local machine", action='store_true')
parser.add_argument("--dumppings",        help="Transfer only pings to local machine", action='store_true')

parser.add_argument("--run",        type=str,   help="Run new instances", action='store', nargs=1)
parser.add_argument("--terminate",         help="Terminate all instances", action='store_true')

parser.add_argument("--nolimit",        help="Do not limit bandwidth", action='store_true')
parser.add_argument("--limitrate",        type=str,   help="Limit the bandwidth per node to PARAM Mbps", action='store', nargs=1)

args = parser.parse_args()


if args.nolimit:
    impose_limit = False

if args.limitrate:
    impose_limit = True
#    rate_Mbps = int(args.limitrate[0])
    rate_Mbps = args.limitrate[0]

if args.list:
    list_instances( instances )

if args.getmax:
    print('='*80+'\n'+ 'Max instances per city : ')
    full_regions =  query_regions_for_max_instance( full_regions )
    for r in full_regions:
        print('%15s' % full_regions[r]['city'] +  " " + full_regions[r]['max_instances'] if 'max_instances' in full_regions[r] else '0' )


if args.boot:
        bootstrap_all_instances( instances, False )
        check_bootstraped( instances )

if args.bootover:

    answer = input("Are you sure you want to BOOTSTRAP OVER all instances? (yes/anything):")
    if answer == 'yes': 
        bootstrap_all_instances( instances )
        check_bootstraped( instances )

if args.checkboot:
    check_bootstraped( instances )

if args.launch:
#    answer = input("Are you sure you want to RE-LAUNCH all nodes (yes/anything):")
    answer="yes"
    if answer == 'yes': 
        kill_launched_nodes( instances )
        relaunch_nodes( instances, impose_limit, rate_Mbps )
        get_running_nodes( instances )

if args.speciallaunch:
    #answer = input("Are you sure you want to RE-LAUNCH all nodes (yes/anything):")
    #if answer == 'yes': 
        kill_launched_nodes( instances )
        special_relaunch_nodes( instances, impose_limit, rate_Mbps )
        get_running_nodes( instances )


if args.killnodes:

#    answer = input("Are you sure you want to KILL all running nodes on instances? (yes/anything):")
    answer="yes"
    if answer == 'yes': 
        kill_launched_nodes( instances )
        get_running_nodes( instances )


if args.compare:
    print_hashes( instances )

if args.compareon:
    print_hashes( instances, int(args.compareon[0]) )

if args.pattern:
    check_pattern( instances, args.pattern[0] )

if args.dlat:
    get_dlat( instances )


if args.nodes:
    get_running_nodes( instances )

if args.dump:
#    answer = input("Are you sure you want to DUMP the data from all nodes to local hdd (yes/anything):")
    answer = 'yes';
    if answer == 'yes': 
        dump_data( instances )

if args.dumpblocks:
#    answer = input("Are you sure you want to DUMP BLOCKS from all nodes to local hdd (yes/anything):")
    answer="yes"
    if answer == 'yes': 
        dump_blocks( instances )

if args.dumppings:
    dump_pings( instances )


if args.run:

    answer = input("Are you sure you want to RUN NEW INSTANCES (yes/anything):")
    if answer == 'yes': 
        run_new_instances( instances, int(args.run[0]) )
        list_instances( instances )


if args.terminate:

    answer = input("Are you sure you want to TERMINATE ALL instances (yes/anything):")
    if answer == 'yes': 
        terminate_all_instances( instances)
        list_instances( instances )




