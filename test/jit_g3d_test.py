import os,time
pid = os.getpid()

import g3d
import pyjit

@pyjit.func(lang="cpp")
def test_func(g3d):
    """
    g3d["Connect"]("olivia");
    int s = g3d["IsConnected"]();
    printf("OK:%d\n",s);
    return 0;   
    """
if __name__ == '__main__':
    test_func(g3d)
    os.getcwd()