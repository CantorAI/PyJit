import os,time
pid = os.getpid()
import pyjit


#os.chdir("C:\\Dev\\PyJit\\buildtest")
srcs=["helloworld.cpp"]
#srcs=["*.cpp>[cppcompiler.cpp,buildsystem.cpp]"]
src =pyjit.expand(srcs)

target_name ="hello_pyjit"
output_dir ="."
target_type ="exe"
target_lang = "c++"
m1 = [("YAML_CPP_STATIC_DEFINE",None),("WIN32",1)]

def direct_build():
    from distutils import ccompiler
    compiler = ccompiler.new_compiler()
    from distutils import sysconfig
    sysconfig.customize_compiler(compiler)

    output_file ="C:\\Dev\\PyJit\\buildtest\\out\\"+target_name
    objs = compiler.compile(srcs,debug=True,output_dir=output_dir,macros = m1,extra_preargs=["/std:c++17"])
    ret = compiler.link_executable(objs,output_file)

#direct_build()

pyjit.build(srcs=srcs,
            target_name=target_name,
            target_type = target_type,
            target_lang = target_lang,
            debug=True,
            #macros = m1,
            #link_extra_preargs=["-lstdc++"],
            output_dir=output_dir)

print("end")


