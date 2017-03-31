
#!/bin/bash

count=1
hundred=32
path="/home/liutao/backup_data/"
for path1 in ${path}/*.8kb.hash.anon
do
    sudo ./destor "${path1}" 
    # sudo ./destor -g0
    if [ $count -eq $hundred ]
    then
        break
    fi
    let count+=1
done
sudo ./destor -g4
sudo ./destor -g9
sudo ./destor -g14
exit
