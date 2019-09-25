import OpenDLA 

def OnStart():
    OpenDLA.addPoint((0,0,0),(1,0,0))
    OpenDLA.addPoint((0,100,0),(0,0,0))
    OpenDLA.addPoint((0,0,100),(0,2,0))
    OpenDLA.addPoint((-100,0,0),(1,1,1))
    pass

def OnStep():
    return (1, 1)


#def OnCollide():
#    pass

# Returns start pos of the new walker
#def OnSpawn():
#    return (0, 0, 0)