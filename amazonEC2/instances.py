def is_running_instance( ins ):
    if 'State' in ins and 'Name' in ins['State']:
        return ins['State']['Name'] == 'running'

    return False

def get_instance_ip( ins ):
    if is_running_instance( ins ) and 'PublicIpAddress' in ins:
        return ins['PublicIpAddress']
    return ''   

def get_instance_ip_private( ins ):
    if is_running_instance( ins ) and 'PrivateIpAddress' in ins:
        return ins['PrivateIpAddress']
    return ''  

def get_instance_region( ins ) :
    return ins['myregion'] 

def get_instance_key( ins ): 
    return ins['mykey']

def get_instance_city( ins ): 
    return ins['mycity']