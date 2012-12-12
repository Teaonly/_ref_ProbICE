mytarget = "linux"
def PrintPath(fileList):
    for i in range(0, len(fileList)):
        if ( (fileList[i].find("base/") == 0) or\
             (fileList[i].find("xmpp/") == 0) or\
             (fileList[i].find("p2p/") == 0) or\
             (fileList[i].find("xmllite/") == 0) or\
             (fileList[i].find("session/tunnel/") == 0) or\
             (fileList[i].find("examples/login/") == 0) ):
            if (fileList[i].endswith(".cc") == False and fileList[i].endswith(".c") == False):
                continue
            global mytarget
            if ( mytarget == "android" or mytarget == "linux" or mytarget == "mac" or mytarget == "ios"):
                if ( fileList[i].find("window") == -1):
                    print "\tlibjingle-0.6.14/talk/" + fileList[i] + "\\"
            else:
                fl = fileList[i].replace("/", "\\");
                print "<File"
                print '\tRelativePath="..\\talk\\' + fl + '"'
                print ">"
                print "</File>"        

def Library(env="", name="", \
        libs="", lin_libs="", posix_libs="", mac_frameworks = "", win_libs="",\
        srcs="", lin_srcs="", posix_srcs="", win_srcs="", mac_srcs="", linphone_srcs="", extra_srcs="", \
        includedirs="", \
        cppdefines="", posix_cppdefines="", win_cppdefines="", \
        win_ccflags="", mac_ccflags="",\
        lin_packages="", dependent_target_settings=""\
        ):
    
    if ( name == "jingle" ):
        PrintPath(srcs)
        if ( (mytarget == "linux") or (mytarget == "android") ):
            PrintPath(lin_srcs)
            PrintPath(posix_srcs)
        elif ( mytarget == "win"):
            PrintPath(win_srcs)      
        elif ( mytarget == "mac"):
            PrintPath(posix_srcs)
            PrintPath(mac_srcs)  
        elif ( mytarget == "ios" ):
            PrintPath(posix_srcs)

    if ( name == "xmpphelp"):
        PrintPath(srcs)

def App(env="", name="",\
        libs="", lin_libs="", posix_libs="", mac_frameworks = "", win_libs="",\
        srcs="", lin_srcs="", posix_srcs="", win_srcs="", mac_srcs="", linphone_srcs="", extra_srcs="", \
        includedirs="", \
        cppdefines="", posix_cppdefines="", win_cppdefines="", \
        win_ccflags="", mac_ccflags="",\
        lin_packages="", dependent_target_settings=""\
        ):
    pass;   

def Unittest(env="", name="",\
        libs="", lin_libs="", posix_libs="", mac_frameworks = "", mac_FRAMEWORKS = "", mac_libs="", win_libs="",\
        srcs="", lin_srcs="", posix_srcs="", win_srcs="", mac_srcs="", linphone_srcs="", extra_srcs="", \
        includedirs="", \
        cppdefines="", posix_cppdefines="", win_cppdefines="", \
        win_ccflags="", mac_ccflags="",\
        lin_packages="", dependent_target_settings=""\
        ):
    pass;      
