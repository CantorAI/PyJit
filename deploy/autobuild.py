import os,sys,subprocess,shutil
from pathlib import Path,PureWindowsPath,PurePosixPath
import glob
import docker
from docker.types import containers
from datetime import date
import yaml



def run(cmd):
    proc = subprocess.Popen(cmd,
                            stdout = subprocess.PIPE,
                            stderr = subprocess.STDOUT,
                            universal_newlines = True
                            )
    while proc.poll() is None:
        line = proc.stdout.readline()
        if line != "":
            print(line, end='')
 
def build_windows_version(cfg,curFolder,py_cmd,dist_out_subfolder):
    os.environ["RELEASE_VERSION"] = cfg.cur_version
    setup_py =cfg.curFolder.joinpath('setup.py')
    rel_root_path = PureWindowsPath(cfg.rootfolder)
    outFolder =rel_root_path.joinpath(cfg.release_dayfolder,'windows',dist_out_subfolder)
    run([py_cmd,str(setup_py),'bdist_wheel','--dist-dir',outFolder])
    #check if wheel is there
    wheel_files = glob.glob(os.path.join(outFolder, "*.whl"))
    if wheel_files != None and len(wheel_files)==1:
        wheel_file = wheel_files[0]
        print('install:',wheel_file)
        run([py_cmd,'-m','pip','install','--ignore-installed',wheel_file])
        print('run test with:',py_cmd)
        Test_windows_version_wheel(curFolder,py_cmd)


def safe_removedir(curFolder,subfolder):
    folder = os.path.join(curFolder,subfolder)
    if os.path.isdir(folder):
        try:
          shutil.rmtree(folder)
          print('removed folder:{}'.format(subfolder))
        except Exception as e:
          print(e)

def Test_windows_version_wheel(curFolder,py_cmd):
    test_py =curFolder.joinpath('test','jit_test.py')
    run([py_cmd, test_py])


def cleanup(curFolder):
    safe_removedir(curFolder,'build')
    #safe_removedir(curFolder,'dist')
    safe_removedir(curFolder,'out')
    safe_removedir(curFolder,'pyjit.egg-info')

#docker /hostfs must be mapped to c:/ or d:/
def path_from_windows_to_linux(win_path):
    pathes = os.path.splitdrive(win_path)
    win_folder = pathes[1].strip('\\')
    linux_path = os.path.join('\\','hostfs',win_folder)
    #convert from windows path to Unix path
    linux_path = linux_path.replace("\\","/")
    return linux_path

def build_linux_version(cfg,scriptFolder,container_name,os_tag):           
    linux_path = path_from_windows_to_linux(scriptFolder)
    linux_rel_root = PurePosixPath(cfg.rootfolder_linux_docker)
    outFolder =linux_rel_root.joinpath(cfg.release_dayfolder,os_tag)

    print("build_linux_version:scriptFolder={}\n,outFolder={}\n,linux_path={}".format(scriptFolder,outFolder,linux_path))
    client = docker.from_env()
    container = client.containers.get(container_name)

    _,stream = container.exec_run("dos2unix ./buildwheel.sh",workdir=linux_path,stream=True)
    for data in stream:
        print(data.decode())

    _,stream = container.exec_run("./buildwheel.sh {} {}".format(cfg.cur_version,outFolder),workdir=linux_path,stream=True)
    for data in stream:
        print(data.decode())
    print("Build Linux ->Finish!")

class Config:
    cur_version = ''

    container_name_x86_64 = ''
    container_name_arm64 = ''

    rootfolder = ''
    rootfolder_linux_docker = "/hostfs/Devops/Release"
    release_dayfolder = str(date.today())
    branch_name ='master'
    curFolder ='' #setup.py's folder
    scriptFolder ='' #this script's folder which will be used to find .sh file
    win_pythons =[]

    def __init__(self,**kwargs) -> None:
        self.scriptFolder = Path(__file__).parent
        if 'branch' in kwargs.keys():
            self.branch_name = kwargs["branch"]
        else:
            self.branch_name = 'master'

        self.curFolder = Path(__file__).parent.parent
        cleanup(self.curFolder)

        config_file =None
        if 'config' in kwargs.keys():
            config_file = kwargs["config"]
        if config_file == None:
            config_file = self.scriptFolder.joinpath('config.yml');
        elif not os.path.isabs(config_file):
            config_file = self.scriptFolder.joinpath(config_file);

        self.load(config_file)
        self.rootfolder += "\\"+self.branch_name
        rootfolder_path = PureWindowsPath(self.rootfolder)
        if not os.path.isdir(rootfolder_path):
            os.makedirs(rootfolder_path,exist_ok=True)

        self.rootfolder_linux_docker += "/"+self.branch_name

        self.rootfolder_linux_docker = path_from_windows_to_linux(self.rootfolder)

        rel_root_path = PureWindowsPath(self.rootfolder)
        day_root =rel_root_path.joinpath(self.release_dayfolder)
        if not os.path.isdir(day_root):
            os.makedirs(day_root,exist_ok=True)
        win_root =day_root.joinpath('windows')
        if not os.path.isdir(win_root):
            os.makedirs(win_root,exist_ok=True)
        linux_root =day_root.joinpath('linux')
        if not os.path.isdir(linux_root):
            os.makedirs(linux_root,exist_ok=True)

    def load(self,cfg_file):
        with open(cfg_file) as f:
            data = yaml.load(f, Loader=yaml.FullLoader)
            if 'release' in data:
                rel = data['release']
                if 'version' in rel:
                    self.cur_version =rel['version']
                if 'release_folder' in rel:
                    self.rootfolder =rel['release_folder']
            if 'docker_container' in data:
                docker_container = data['docker_container']
                if 'x86_64' in docker_container:
                    self.container_name_x86_64 =docker_container['x86_64']
                if 'arm64' in docker_container:
                    self.container_name_arm64 =docker_container['arm64']
            if 'windows_pythons' in data:
                self.win_pythons = data['windows_pythons']



# autobuild.py branch=branch_name config=cfg_file setup=setup.py's path
def main(**kwargs):
    cfg = Config(**kwargs)
    print("autobuild started...")    

    print("[Job 1 Started] for Linux@x86_64")    
    build_linux_version(cfg,cfg.scriptFolder,cfg.container_name_x86_64,"linux_x86_64")
    print("[Job 1 Finished] for Linux@x86_64")    
    print("[Job 2 Started] for Linux@arm64")    
    build_linux_version(cfg,cfg.scriptFolder,cfg.container_name_arm64,"linux_aarch64")
    print("[Job 2 Finished] for Linux@arm64")
    print("[Job 3 Started] for Windows@x86_64")    
    for it in cfg.win_pythons:
        build_windows_version(cfg.curFolder.absolute(),it['cmd'],it['dist'])
    print("[Job 3 Finished] for Windows@x86_64")    
    
    print("Finish autobuild.")
 

if __name__ =='__main__':
    kwargs={kw[0]:kw[1] for kw in [ar.split('=') for ar in sys.argv if ar.find('=')>0]}
    main(**kwargs)

