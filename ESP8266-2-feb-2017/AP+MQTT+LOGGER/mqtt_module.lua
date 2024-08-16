local mqtt_module = {}

mqtt_module.show = 0
mqtt_module.connected=0
mqtt_module.client = nil
mqtt_module.srv_settings_json=nil
mqtt_module.Mid=nil
mqtt_module.controller_parameters=nil

function mqtt_module.init()
	mqtt_module.client = mqtt.Client(node.chipid(),350,"","")
	mqtt_module.client:lwt("/lwt", "Gone offline", 0, 0)
	mqtt_module.client:on("connect", function(client) if mqtt_module.show == 1 then print ("connected") end mqtt_module.connected=1 end)
	mqtt_module.client:on("offline", function(client) if mqtt_module.show == 1 then print ("offline") end mqtt_module.connected=0 end)

	mqtt_module.client:on("message", function(client, topic, data) 
		if mqtt_module.show == 1 then
			print(topic .. ":" ) 
			print(data)
		end
	
		if string.find(data,"s1") then
			mqtt_module.srv_settings_json = cjson.decode(string.match(data,"{.*}")) 
			print("999 "..mqtt_module.srv_settings_json['s1'].." "..mqtt_module.srv_settings_json['s2'].." "..mqtt_module.srv_settings_json['s3'].." "..
			mqtt_module.srv_settings_json['s4'].." "..mqtt_module.srv_settings_json['s5'].." "..mqtt_module.srv_settings_json['s6'].." "..mqtt_module.srv_settings_json['s7'].." "
			..mqtt_module.srv_settings_json['s8'].." "..mqtt_module.srv_settings_json['s9'].." "..mqtt_module.srv_settings_json['s10'].." "..mqtt_module.srv_settings_json['s11'].." "
			..mqtt_module.srv_settings_json['s12'].." "..mqtt_module.srv_settings_json['s13'].." 0".." 0".." 0")

		elseif string.find(data,"updatesettings") then
			print("updatesettings")
			if mqtt_module.controller_parameters ~= nil then
				mqtt_module.sendToBroker(mqtt_module.controller_parameters)
			else
				mqtt_module.sendToBroker("ERROR: Settings not found")
			end
		end

	end)
end

function mqtt_module.connectToBroker()
	if(mqtt_module.connected == 0) then
		if mqtt_module.show == 1 then
			print("Connecting..")
		end
			mqtt_module.client:connect("104.168.148.87",1883,0, function(client) 
				if mqtt_module.show == 1 then
					print("Connected")
				end
				mqtt_module.connected = 1
				mqtt_module.client:subscribe(mqtt_module.Mid,0, function(client) uart.write(0,27) print("msg:subscribe success") end)
			end,function(client , reason)
				if mqtt_module.show == 1 then
					print(reason)
				end
			end)
	end
end

function mqtt_module.sendToBroker(data)
if mqtt_module.connected == 1 then 
	mqtt_module.client:publish(mqtt_module.Mid,data,0,0, function(client) 
	if mqtt_module.show == 1 then 
		print("published") 
	end 
	end)
end
end
return mqtt_module