#Q1

Each player/Referee first enters academy
Then waits for the organiser by the shared variable "org_status"
Then each player/referee is allocated his/her group in meetORganiser function
Each player waits until his group is formed 
Once group is formed he/she enters court, warmsup and sends signal that they are ready
Once all are ready,Referee starts the game

Organiser:
Whenever a group is formed, org_status changes to busy
After game starts, org_status changes to free


#Q2

Each booth is an independent thread ( no signals to any other booth)
Each booth has an array of evms and voters
Each booth has condition variables specific to that booth
Each evm is a thread linked to the booth thread 
Each voter is a thread linked to the booth thread
The voter first waits for the evm using cond var "evms" 
Whenever evm is free evm sends signal through "evms"
The allocation process is done parallely in the evm thread
The voter waits for the evm until allocation is done
The voter then casts vote and sends signal through "voters" and voter thread is finished
EVM waits until all its slots of voters are done , allocates slots again until voters are done , then exits





