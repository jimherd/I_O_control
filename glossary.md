# Glossary of relevant terms #
## Stepper motor subsystem ##

|Name|Description|
|---|---|
|profile |  sequence of steps, typically a trapezoidal sequence |
| sm_cmd |set of steps at a fixed speed (i.e. fixed delay between pulses)|
| | Consists of a delay value and a step count |
| |  **profile** consists of a list of **sm_cmd**'s |
| sm_cmd_step_cnt | step count in **sm_cmd** command |                 
| sm_cmd_delay | delay in **sm_cmd** command |
| sm_step | one step of the stepper motor |
| sm_delay | time between stepper motor pulses |
