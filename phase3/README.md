# hi
## run the following commands first ok ya katakeet?

### **Before you start**
``` cmd
cd phase3

sudo apt update
sudo apt install python3-full python3-venv

python3 -m venv venv
source venv/bin/activate

pip install grpcio grpcio-tools opencv-python numpy

#MAKE SURE THIS OUTPUTS A PYTHIN PATH INSIDE VENV ex. /home/asus/image-processing/phase3/venv/bin/python

which python (da el command to see)

#if not go to section debug python

#this should show grpcio w grpcio-tools, numpy, protobuf
pip list

```
### **Python debug**
if youre here, damn
bas try this ig
1- **Ctrl + Shift + P**
2- **Python: Select Interpreter**
3- Choose: **"./venv/bin/python"**

check again if it works, law la2 idk da a5ry


### **To run** (Note: ***each command of the 3 in a diff terminal and in the same venv***) make the venv once, and activate it in the terminals
``` xmd
cd src
1)
python server.py 50051 
2)
python server.py 50052
3)
python client.py
```

this is a complete flow from a new terminal
``` cmd
cd phase3

source venv/bin/activate

which python

#should show ../phase3/venv/python

cd src

python {command: ex. server.py 50051, server.py 50052, client.py}
```

### Failures
There are 2 ways to inject failures
1- server crash
    **Explanation**
        Server failing and crashing
    **Usage**
    - either manually (stop a server mid running) or
    - i added a way to make a server autofail after a specified period in failing_server.py
        To use it, instead of starting a server using
        ```cmd
            python server.py {port}
        ```

        use this
        ``` cmd
            python failing_server.py {port} --kill {optional: --count (time in secs), deafult 10 secs, --recovery (time to recover)}

            #example usage
            python failing_server.py 50051 --kill --count 5 --recovery 5
        ```

2- client timeout
    **Explanation**
        timeout is very small, simulating a slow network
    **Usage**
        instead of
        ```cmd
            python client.py
        ```

        use this
        ```cmd
            python failing_client.py
        ```

for spark:
sudo apt update
sudo apt install openjdk-17-jdk
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
export PATH=$JAVA_HOME/bin:$PATH

