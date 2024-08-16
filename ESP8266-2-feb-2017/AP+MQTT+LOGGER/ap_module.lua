local ap_module = {}

ap_module.show = 0
ap_module.authenticated=0
ap_module.info = {}
ap_module.values=nil
ap_module.values_str=""
ap_module.session=nil
ap_module.timeout = 0
ap_module.controller_parameters=nil
ap_module.srv_settings_json=nil

function ap_module.save_settings(value)
	if file.open("setting.lua","w+") then
		file.write(value)
		file.close()
	else
		--print("Could not Open File !")
	end
end
function ap_module.read_settings(info)
	if file.open("setting.lua") then
		result = file.read()
		file.close()
		for word in result:gmatch("%S+") do table.insert(info, word) end
	else
		--print("Could not Open File !")
	end
end

local function makeString(l)
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
local function update()
	conn=net.createConnection(net.TCP, 0) 
	conn:connect(80,'192.168.1.111')
	conn:on("connection",function()

			uart.write(0,27)
			i=vars["d1"]+math.random(20)
			vars["d2"]=vars["d2"]+(i/6)
			print("msg: Sending Data..\r\n")
			conn:send("GET /demo/login.php?name="..info[2].."&password="..info[3].."&meter_id="..info[5].."&flow="..i.."&total="..vars["d2"]
					.. " HTTP/1.1\r\n"
					.. "Host: 192.168.1.111\r\n"
					.. "Connection: keep-alive\r\n"
					.. "Accept: */*\r\n"
					.. "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n"
					.. "\r\n")
			print(i.." "..vars["d2"]+i/6)
			uart.write(0,27)
			print("msg: Sending Meter ID..\r\n")
	end)
	conn:on("receive", function(conn, pl) 
		--if ap_module.show==1 then
			print(pl)
		--end
	if string.find(pl,"user_id") then
		print("DATA SENT\r\n")
	elseif string.find(pl,"Login unsuccessfull!") then
		print("User ID does not exist\r\n")
		
	elseif string.find(pl,"Invalid Session ID") then
		Authentication = 0
	elseif string.find(pl,"Data Send Successfully....!!!!") then
		count = 1
		Authentication = 0
		Data_Flag=0
		session=0
		print("msg: Data Sent..\r\n")
			tmr.stop(2)
			tmr.alarm(0, 1000, 1, function() ap_module.schedule() end)
	end
	end)
	conn:on("sent",function(conn, payload)
		if ap_module.show==1 then
			print("Sent !\n")
		end
	end)	
	conn:on("disconnection",function(conn) 
		if ap_module.show==1 then
			print("Disconnected")
		end
	end)

end
function ap_module.schedule() 
	
	if ap_module.timeout == 3600 or ap_module.timeout == 0 then
		tmr.alarm(2, 10000, 1, function() 
			if ap_module.values ~= nil then
				update()
			else
				if ap_module.show==1 then
					print("No Data to Send ....!")
				end
			end
		end)
		tmr.stop(0)
	else
		ap_module.timeout = ap_module.timeout+1
	end
end
function ap_module.init()
	ap_module.server = net.createServer(net.TCP)
	ap_module.server:listen(80,'192.168.4.1',function(scon) 
		scon:on("receive", function(client,request)
		if string.find(request,"updatesettings") then
			print(request)
			if ap_module.controller_parameters ~= nil then
				client:send(ap_module.controller_parameters.."\r\n")
			else
				client:send("<h3>ERROR: Settings not found</h3>\r\n")
			end
		elseif string.find(request,"updatevalues") then
			if ap_module.values_str ~= "" then
				client:send(ap_module.values_str.."\r\n")
			else
				client:send("<h3>ERROR: Data not available</h3>\r\n")
			end	
		elseif string.find(request,"s1") then
				uart.write(0,27)
				ap_module.srv_settings_json = cjson.decode(string.match(request,"{.*}")) 
				print("999 "..ap_module.srv_settings_json['s1'].." "..ap_module.srv_settings_json['s2'].." "..ap_module.srv_settings_json['s3'].." "..
				ap_module.srv_settings_json['s4'].." "..ap_module.srv_settings_json['s5'].." "..ap_module.srv_settings_json['s6'].." "..ap_module.srv_settings_json['s7'].." "
				..ap_module.srv_settings_json['s8'].." "..ap_module.srv_settings_json['s9'].." "..ap_module.srv_settings_json['s10'].." "..ap_module.srv_settings_json['s11'].." "
				..ap_module.srv_settings_json['s12'].." "..ap_module.srv_settings_json['s13'].." 0".." 0".." 0")
				client:send("<h3>Sent !</h3>\r\n")
     	elseif string.find(request,"ssid") then
	 		local raw_inf = string.match(request,"{.*}") 
	 		local d_inf = cjson.decode(raw_inf)
			wifi.sta.config(d_inf["ssid"],d_inf["pwd"])
			client:send("<h3>OK</h3>\r\n")
		elseif string.find(request,"userinfo>") then	
		ap_module.save_settings(request)
		client:send("<h3>Settings saved</h3>\r\n")
		else
			client:send("<h3>Unrecognised command !</h3>\r\n")
			print(request)
	 	end
	
	end)
		scon:on("sent",function(scon) scon:close() end)
	end)
end





return ap_module