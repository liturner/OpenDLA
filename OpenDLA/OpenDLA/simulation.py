def OnUpdate(a,b):
    print("Will compute", a, "times", b)
    c = 0
    for i in range(0, a):
        c = c + b
    return (1, 0, 0)

#def OnCollide():
#    pass

# Returns start pos of the new walker
#def OnSpawn():
#    return (0, 0, 0)