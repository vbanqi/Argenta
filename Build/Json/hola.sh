#!/bin/bash
#python generateParser.py conf.json rapidjson Interact
#python generateStruct.py conf.json Interact
#cp Metadata* ../Poseidon/Src/Protocol/Interact/
objpath=../jsonobj/
rm -rf ${objpath}

python generateStruct.py dispatch.json hola ProtocolData ${objpath} 
python generateParser.py dispatch.json rapidjson hola ProtocolData ${objpath}

#python generateStruct.py relayReport.json PoseidonCore ReportData 256 300 ${objpath} 
#python generateParser.py relayReport.json rapidjson PoseidonCore ReportData ${objpath}

rm -rf ../../Src/Protocol/Model/Protocol*

cp ${objpath}Protocol* ../../Src/Protocol/Model/
