# STM32BL475-IOT-dico1 board template 
This is a template project demonstrating how to use the STM32BL475 IoT Discovery board.

- The template focuses on using BLE (Bluetooth Low Energy).
- You can also use STM32CubeMX for pinout and peripheral configuration (when generating the code, make sure to configure CubeMX to generate pinouts and peripheral configurations separately).
- A Makefile is provided for building and flashing the project.
- This project is VS Code–friendly: you can open it directly in VS Code. IntelliSense has been configured in the .vscode/ directory.

> I also recomment taking a look at the BSP under `Drivers/BSP`
> You will find there the related APIs for the BLE and also for the onboard sensors 

- The current main runs a simple BLE application, a notification when the on board btn is pushed and LED service control it. 
- I have used the `NRF Connect` mobile application to test this.
## Usage 

- Buidling 
```bash 
make build
```
- Flashing 
```bash 
make flash
```

## Dependencies
- make 
- stlink-tools 
- gcc-arm toolchain
```bash 
sudo apt install make stlink-tools gcc-arm-none-eabi
```


