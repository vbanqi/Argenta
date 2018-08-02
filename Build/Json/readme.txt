# generate json file frome struct
# or python generate.py struct2.txt
# param struct.txt: the struct file
python generate.py struct.txt

# generate struct from json
# This script will generate struct.h file
# params cache ：the json file
python generateStruct.py cache

#generate parser.h and parser.cpp
# params cache: the json file
# params rapidjson: the rapidjson rilative path
# param Interact: the namespace
python generateParser.py cache rapidjson Interact

python generateParser1.py dispatch_heartbeat.json rapidjson Interact

python generateStruct1.py dispatch_heartbeat.json Interact DispatchHeartBeat
