import subprocess
import os
import shutil

def exec(args, no_output = False):
    process = subprocess.run(args, shell=True, capture_output=no_output)
    return process.returncode == 0

print("Checking docker Image...")
if not exec("docker image inspect emsdk", True):
    print("Creating docker image...")
    exec("docker build -t emsdk-minimal-wgpu .")

workdir=os.path.dirname(os.getcwd())
result = exec(f"""docker run --rm -v "{workdir}:/work" emsdk-minimal-wgpu sh -c "cd /work/wasm; emcmake cmake -DCMAKE_BUILD_TYPE=Debug ..; emmake make" """)
