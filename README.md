# MyGSMCenter

This is a simple Arduino GSM gateway software.
Convert AT commands to simple line instructions and communicate ower serial port.
I used Arduino Leonardo for open two COM ports. One for commands another for communicate with the SIM900 shield.
The Eventually library is needed for task scheduling.

Commands that can send ower serial port:
CALL:+36301234567 - start outgoing call
SMS:+36301234567:message - send message
BALANCE - get user balance
ANSWER - answer incoming call
HANGUP - hangup call or ring
