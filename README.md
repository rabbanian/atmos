# atmos
a temperature monitoring system based on arduino.  
Atmos reads three sensor, calculates average temperature and removes
the sensor which has a wrong value.  
It also shows simple messages:  
`Temp High` if temperature is higher than the top threshold.  
`Temp Low` if temperature is lower than the down threshold.  
`Temp OK` if temperature is in between top and down thresholds.  
the thresholds can be configured via settings menu.  
# wiring

| Arduino Port Number   |      Part      |
|----------|:-------------:|
| 13 |  LCD(D7) |
| 12 |  LCD(D6) |
| 11 |  LCD(D5) |
| 10 |  LCD(D4) |
| 9 |  LCD(E) |
| 8 |  LCD(RS) |
| 7-0 |  KeyPad |
| A0-A2 |  Sensors |

# settings menu
Entering settings menu can be done by pressing `A`. Atmos will ask a password, default is `123467`.
By pressing `B` or `C` thresholds can be configured, decimal values are accepted. At any state `A` act as `Cancel`, `D` as `Enter` and `B` as `Back`.
