import inspect
import sys

for i in range(1,8):
    frms = sys._getframe(i)
    if frms == None:
        break
    fileName = frms.f_code.co_filename
    print(fileName,"\n")
    fileName =inspect.getfile(frms)
if getattr(frms, '__file__', None):
    fileName =  frms.__file__

for i in frms:
    frm = frms[i]
    name = frm.filename
    print("name=",name,"\n")
print("OK")