# Glossary of relevant terms #
## Stepper motor subsystem ##

|Name|Description|
|---|---|
|profile |  sequence of **sm_cmd**'s, typically a trapezoidal sequence |
| sm_cmd |set of steps at a fixed speed (i.e. fixed delay between pulses)|
| | Consists of a delay value and a step count |
| |  **profile** consists of a list of **sm_cmd**'s |
| sm_cmd_step_cnt | step count in a single **sm_cmd** command |                 
| sm_cmd_delay | delay between pulses in a single **sm_cmd** command |
| sm_step | one step of the stepper motor |
| sm_delay | time between stepper motor pulses |
