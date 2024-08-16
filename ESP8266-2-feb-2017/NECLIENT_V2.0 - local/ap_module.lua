local ap_module = {}

ap_module.show = 0
ap_module.authenticated=0
ap_module.info = {}
ap_module.values=nil
ap_module.values_str=""
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