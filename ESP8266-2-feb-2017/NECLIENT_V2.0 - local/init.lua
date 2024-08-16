local apClient = require "ap_module"
Authentication = 0
Data_Flag=0
vars = nil
count = 0
cfg = {}
i=0
info = {}
show = 0
count2=250
count1 =1000
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
function makeString(l)
		math.randomseed(tmr.now())
        if l < 1 then return nil end 
        local s = "" 
        for i = 1, l do
            n = math.random(32, 126) 
            if n == 96 then n = math.random(32, 95) end
                s = s .. string.char(n) 
        end
        return s
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
			show=1
		else
	 		print("Unrecognized !")
	 	end
	 end       
 end,0)

function update()
	conn=net.createConnection(net.TCP, 0) 
	conn:connect(8080,'172.16.0.110')
	conn:on("connection",function()

			uart.write(0,27)
			print("msg: Sending Data..\r\n")
			conn:send("GET /alkaram/api/login.php?name="..apClient.info[2].."&password="..apClient.info[3].."&meter_id="..apClient.info[5]
					.."&flow="..apClient.values["d3"].."&total="..apClient.values["d2"]
					.."&temp="..apClient.values["d1"]
					.."&pressure="..apClient.values["d4"]
					.."&d5="..apClient.values["d5"]
					.."&d6="..apClient.values["d6"]
					.."&d7="..apClient.values["d7"]
					.."&d8="..apClient.values["d8"]
					.. " HTTP/1.1\r\n"
					.. "Host: 172.16.0.110\r\n"
					.. "Connection: keep-alive\r\n"
					.. "Accept: */*\r\n"
					.. "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n"
					.. "\r\n")
			
end)

			
conn:on("receive", function(conn, pl) 
	if show ==1 then
		print(pl)
	end
	
	if string.find(pl,"Data Send Successfully....!!!!") then
		uart.write(0,27)
		print("msg: DATA SENT\r\n")
	elseif string.find(pl,"Login unsuccessfull!") then
		uart.write(0,27)
		print("msg: User ID does not exist\r\n")
		
	elseif string.find(pl,"Invalid Session ID") then
		Authentication = 0
		count = 1
		Authentication = 0
		Data_Flag=0
		session=0
		uart.write(0,27)
		print("msg: Data Sent..\r\n")
		--tmr.stop(2)
		--tmr.alarm(0, 1000, 1, function() schedule() end)
	end
end)
conn:on("sent",function(conn, payload)
		if show==1 then
			print("Sent !\n")
		end
end)	
conn:on("disconnection",function(conn) 
		if show==1 then
			print("Disconnected")
		end
end)
end

function schedule() 
	
	if count == 60 or count == 0 then
		--tmr.alarm(2, 10000, 1, function() 
			if apClient.values ~= nil then
				update()
				count=1
			else
				if show==1 then
					print("No Data to Send ....!")
				end
			end
		--end)
		--tmr.stop(0)
	else
		count = count+1
	end
end

wifi.sta.eventMonReg(wifi.STA_IDLE, function() print("msg:STATION_IDLE") end)
wifi.sta.eventMonReg(wifi.STA_CONNECTING, function() print("msg:STATION_CONNECTING")tmr.stop(0) tmr.stop(1) tmr.stop(2) end)
wifi.sta.eventMonReg(wifi.STA_WRONGPWD, function() print("msg:STATION_WRONG_PASSWORD") end)
wifi.sta.eventMonReg(wifi.STA_APNOTFOUND, function() print("msg:STATION_NO_AP_FOUND") end)
wifi.sta.eventMonReg(wifi.STA_FAIL, function() print("msg:STATION_CONNECT_FAIL") node.restart() end)
wifi.sta.eventMonReg(wifi.STA_GOTIP, function() uart.write(0,27) print("msg:STATION_GOT_IP") 
	tmr.stop(0)
	tmr.stop(1)
	tmr.stop(2)
	tmr.alarm(0, 1000,tmr.ALARM_AUTO, function() schedule() end)
end)




wifi.sta.eventMonStart(5000)