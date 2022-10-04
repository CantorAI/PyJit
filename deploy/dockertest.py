import docker
from docker.types import containers
client = docker.from_env()
container_name = 'manylinux2014-buildmachine'

container = client.containers.get(container_name)

res = container.exec_run("ls")
print(res.output)
