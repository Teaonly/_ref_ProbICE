import sys
import talk

def Import(arg):
    pass

talk.mytarget = "linux"  #android, ios, linux, mac, win
if len(sys.argv) == 2:
    talk.mytarget = sys.argv[1]
env = ""
