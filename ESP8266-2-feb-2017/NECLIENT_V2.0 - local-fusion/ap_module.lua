local ap_module = {}

ap_module.show = 0
ap_module.authenticated=0
ap_module.info = {}
ap_module.values_str=""
ap_module.values_str1=""
ap_module.values_str2=""
ap_module.values_commissioning=""
ap_module.values_commissioning2=""
ap_module.values_commissioning3=""
ap_module.values_feild=""
ap_module.values_steam=""
ap_module.values_oxygen=""
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
		if string.find(request,"fupdatevalues") then
			if ap_module.values_str ~= "" then
				client:send(ap_module.values_str.."\r\n")
				print("101\r\n")
			else
				client:send("<h3>ERROR: Data not available</h3>\r\n")
			end
		elseif string.find(request,"supdatevalues") then
			if ap_module.values_str1 ~= "" then
				client:send(ap_module.values_str1.."\r\n")
			else
				client:send("<h3>ERROR: Data not available1</h3>\r\n")
			end
		elseif string.find(request,"tupdatevalues") then
			if ap_module.values_str2 ~= "" then
				client:send(ap_module.values_str2.."\r\n")
			else
				client:send("<h3>ERROR: Data not available2</h3>\r\n")
			end
		elseif string.find(request,"commission") then
			print("commission")
			if ap_module.values_commissioning ~="" then
				client:send(ap_module.values_commissioning.." "..ap_module.values_commissioning2.." "..ap_module.values_commissioning3.."\r\n")
				--client:send(ap_module.values_commissioning2.."\r\n")
				--client:send(ap_module.values_commissioning3.."\r\n")
			else
				client:send("<h3>ERROR: Commissioning Data not available<h3>\r\n")
			end
		elseif string.find(request,"feild") then
			print("feild")
			if ap_module.values_feild ~="" then
				client:send(ap_module.values_feild.."\r\n");
			else
				client:send("<h3>ERROR: Feild Setup Data not available<h3>\r\n")
			end
		elseif string.find(request,"steam") then
			print("steam")
			if ap_module.values_steam ~="" then
				client:send(ap_module.values_steam.."\r\n");
			else
				client:send("<h3>ERROR: Feild Setup Data not available<h3>\r\n")
			end
		elseif string.find(request,"oxygen") then
			print("oxygen")
			if ap_module.values_oxygen ~="" then
				client:send(ap_module.values_oxygen.."\r\n");
			else
				client:send("<h3>ERROR: Oxygen Scale not available<h3>\r\n")
			end
		elseif string.find(request,"inst") then
			uart.write(0,27)
			print(" "..request)
			client:send("<h3>Sent !</h3>\r\n")
		elseif string.find(request,"999") then
				uart.write(0,27)
				print(" "..request)
				client:send("<h3>Sent !</h3>\r\n")
		elseif string.find(request,"set1") then
				uart.write(0,27)
				print(" "..request)
				client:send("<h3>Sent !</h3>\r\n")
		elseif string.find(request,"cet1") then
				uart.write(0,27)
				print(" "..request)
				client:send("<h3>Sent !</h3>\r\n")
		elseif string.find(request,"oxy") then
			uart.write(0,27)
			print(" "..request)
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