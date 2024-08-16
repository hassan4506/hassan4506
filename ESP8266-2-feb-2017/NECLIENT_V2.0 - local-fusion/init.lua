local apClient = require "ap_module"
Authentication = 0
Data_Flag=0
vars = nil
count = 0
cfg = {}
i=0
info = {}
show = 0
i=0
session =nil
wifi.setphymode(wifi.PHYMODE_B)
wifi.setmode(wifi.STATION)
apClient.read_settings(apClient.info)
if apClient.info[5] == nil then
	apClient.info[1]="settings"
	apClient.info[2]="HUNCH"
	apClient.info[3]="hunch"
	apClient.info[4]="18"
	apClient.info[5]="5000000103"
end
wifi.setmode(wifi.STATIONAP)
cfg.ssid=apClient.info[5]
cfg.pwd="hunch5400"
wifi.ap.config(cfg)
apClient.init()

uart.setup(0, 9600, 8, uart.PARITY_NONE, uart.STOPBITS_1, 1)
uart.on("data", "\r",function(data)
     if data ~= nil then
     	
     	if string.find(data,"settings>") then
			local i = string.find(data,"settings>")
			apClient.controller_parameters = string.sub(data,i)
     	elseif string.find(data,"userinfo>") then	
			for word in data:gmatch("%S+") do table.insert(apClient.info, word) end
			apClient.save_settings(data)
     	elseif string.find(data,"d0") then
	 		apClient.values_str = string.match(data,"{.*}")
			apClient.values = cjson.decode(apClient.values_str)
		elseif string.find(data,"d10") then
			apClient.values_str1 = string.match(data,"{.*}")
		elseif string.find(data,"d20") then
			apClient.values_str2 = string.match(data,"{.*}")
		elseif string.find(data,"c0") then
			apClient.values_commissioning = string.match(data,"{.*}")
		elseif string.find(data,"c12") then
			apClient.values_commissioning2 = string.match(data,"{.*}")
		elseif string.find(data,"c24") then
			apClient.values_commissioning3 = string.match(data,"{.*}")
		elseif string.find(data,"f0") then
			apClient.values_feild = string.match(data,"{.*}")
		elseif string.find(data,"s0") then
			apClient.values_steam= string.match(data,"{.*}")
		elseif string.find(data,"o1") then
			apClient.values_oxygen= string.match(data,"{.*}")	
		elseif string.find(data,"getout") then
			print("Command Mode !")
	 		uart.on("data")
	 	elseif string.find(data,"info") then
			print(apClient.info[2])
			print(apClient.info[3])
			print(apClient.info[4])
			print(apClient.info[5])
		elseif string.find(data,"show") then
			apClient.show = 1
			show=1
		else
	 		print("Unrecognized !")
	 	end
	 end       
 end,0)


