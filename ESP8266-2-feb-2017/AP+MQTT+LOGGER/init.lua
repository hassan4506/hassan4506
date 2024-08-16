local apClient = require "ap_module"
cfg = {}
wifi.setphymode(wifi.PHYMODE_B)
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
apClient.show = 0

uart.setup(0, 9600, 8, uart.PARITY_NONE, uart.STOPBITS_1, 0)
uart.write(0,27)
uart.write(0,27)
uart.write(0,27)
uart.on("data", "\r",function(data)
     if data ~= nil then
		if string.find(data,"settings>") then
			local i = string.find(data,"settings>")
			apClient.controller_parameters = string.sub(data,i)
			mqClient.controller_parameters = apClient.controller_parameters;
     	elseif string.find(data,"userinfo>") then	
			for word in data:gmatch("%S+") do table.insert(apClient.info, word) end
			apClient.save_settings(data)
     	elseif string.find(data,"d1") then
	 		apClient.values_str = string.match(data,"{.*}")
			apClient.values = cjson.decode(apClient.values_str)
			
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
			mqClient.show = 1
		else
	 		print("Unrecognized !")
	 	end
	 end       
 end,0)
wifi.sta.eventMonReg(wifi.STA_IDLE, function() print("msg:STATION_IDLE") end)
wifi.sta.eventMonReg(wifi.STA_CONNECTING, function() print("msg:STATION_CONNECTING")tmr.stop(0) tmr.stop(1) tmr.stop(2) end)
wifi.sta.eventMonReg(wifi.STA_WRONGPWD, function() print("msg:STATION_WRONG_PASSWORD") end)
wifi.sta.eventMonReg(wifi.STA_APNOTFOUND, function() print("msg:STATION_NO_AP_FOUND") end)
wifi.sta.eventMonReg(wifi.STA_FAIL, function() print("msg:STATION_CONNECT_FAIL") node.restart() end)
wifi.sta.eventMonReg(wifi.STA_GOTIP, function() uart.write(0,27) print("msg:STATION_GOT_IP")end)
	tmr.stop(0)
	tmr.stop(1)
	tmr.stop(2)
	
	apClient.timeout=0
	
	tmr.alarm(0, 1000, 1, function()  apClient.schedule() end)
	
wifi.sta.eventMonStart(5000)
