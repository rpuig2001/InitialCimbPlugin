# Initial Climb plugin
InitialClimbPlugin is an Euroscope plugin that allows us to see the initial climb depending on the airport, runway and departure.

## How to use:
- Load up the plugin.
- Add Tag Item type & function to Departure List.
- Edit 'initialClimb.xml' config file located in the InitialClimbPlugin folder.
![image](https://user-images.githubusercontent.com/68125167/133853455-9ffd44e3-5ebc-46a8-bfe5-a9328ebc1e82.png)

### Define configurations
- Add icao (ex. apt icao="LEAL").
- Runway for defined airport (ex. runway name="10").
- Complete SID name (ex. sid name="BAVER3A") or first fix (ex. sid name="BAVER").
- Define altitude for specified sid or fix (ex. alt>040</alt).

## Functions:
- If you click the initial Climb, it will automatically set the Cleared Flight Level (CFL) to the Initial Climb Altitude.
- ![#f03c15](https://via.placeholder.com/15/f03c15/000000?text=+) `RED Altitude`: Cleared Flight Level (CFL) is NOT the same as the Initial Climb.
- ![#78f518](https://via.placeholder.com/15/78f518/000000?text=+) `GREEN Altitude`: Cleared Flight Level (CFL) is the same as the Initial Climb.
- ![#5b6157](https://via.placeholder.com/15/5b6157/000000?text=+) `GREY DASH (-)`: No data from the xml config file.
