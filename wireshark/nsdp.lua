
p_nsdp = Proto("nsdp", "Netgear Switch Description Protocol")

local f_version = ProtoField.uint8("nsdp.version", "Version", base.DEC)
local f_code = ProtoField.uint8("nsdp.code", "Operation Code", base.DEC)
local f_error = ProtoField.uint8("nsdp.error", "Error Code", base.DEC)
local f_errattr = ProtoField.uint16("nsdp.errattr", "Erroneous Attribute", base.HEX)
local f_clientmac = ProtoField.ether("nsdp.clientmac", "Client MAC")
local f_switchmac = ProtoField.ether("nsdp.switchmac", "Switch MAC")
local f_seqnum = ProtoField.uint32("nsdp.seqnum", "Sequence Number", base.DEC)

p_nsdp.fields = {
	f_version,
	f_code,
	f_error,
	f_errattr,
	f_clientmac,
	f_switchmac,
	f_seqnum
}



local op_codes = {
	[1] = "Read Request",
	[2] = "Read Reply",
	[3] = "Write Request",
	[4] = "Write Reply"
}


local error_codes = {
	[0] = "OK",
	[5] = "Invalid Value",
	[7] = "Access Denied"
}


local status_codes = {
	[0] = "down",
	[1] = "10M",
	[4] = "100M",
	[5] = "1000M"
}


local bitrates_codes = {
	[0] = "unlimited",
	[1] = "512K",
	[2] = "1M",
	[3] = "2M",
	[4] = "4M",
	[5] = "8M",
	[6] = "16M",
	[7] = "32M",
	[8] = "64M",
	[9] = "128M",
	[10] = "256M",
	[11] = "512M"
}


local vlan_type_codes = {
	[1] = "port basic",
	[2] = "port advanced",
	[3] = "802.1Q basic",
	[4] = "802.1Q advanced"
}


local qos_type_codes = {
	[1] = "port based",
	[2] = "802.1p"
}


local prio_codes = {
	[1] = "high",
	[2] = "medium",
	[3] = "normal",
	[4] = "low"
}




local function dissect_port_statistics(buffer, offset, subtree)
	subtree:add(buffer(offset + 4, 1), string.format("Port: %i", buffer(offset + 4, 1):uint()))
	subtree:add(buffer(offset + 4 + 1 + 8 * 0, 8), "Received:", tostring(buffer(offset + 4 + 1 + 8 * 0, 8):uint64()))
	subtree:add(buffer(offset + 4 + 1 + 8 * 1, 8), "Sent: ", tostring(buffer(offset + 4 + 1 + 8 * 1, 8):uint64()))
	subtree:add(buffer(offset + 4 + 1 + 8 * 5, 8), "CRC Errors:", tostring(buffer(offset + 4 + 1 + 8 * 5, 8):uint64()))
end


local function dissect_port_status(buffer, offset, subtree)
	local st = buffer(offset + 5, 1):uint()
	subtree:add(buffer(offset + 4, 1), string.format("Port: %i", buffer(offset + 4, 1):uint()))
	subtree:add(buffer(offset + 5, 1), string.format("Status: %i (%s)", st, status_codes[st] or "unk"))
end


local function dissect_qos_type(buffer, offset, subtree)
	local t = buffer(offset + 4, 1):uint()
	subtree:add(buffer(offset + 4, 1), string.format("QoS Type: %i (%s)", t, qos_type_codes[t] or "unk"))
end


local function dissect_qos_config(buffer, offset, subtree)
	local p = buffer(offset + 5, 1):uint()
	subtree:add(buffer(offset + 4, 1), string.format("Port: %i", buffer(offset + 4, 1):uint()))
	subtree:add(buffer(offset + 5, 1), string.format("Priority: %i (%s)", p, prio_codes[p] or "unk"))
end


local function dissect_bitrate(buffer, offset, subtree)
	local sp = buffer(offset + 5, 4):uint()
	subtree:add(buffer(offset + 4, 1), string.format("Port: %i", buffer(offset + 4, 1):uint()))
	subtree:add(buffer(offset + 5, 4), string.format("Speed: %i (%s)", sp, bitrates_codes[sp] or "unk"))
end


local function dissect_vlan_type(buffer, offset, subtree)
	local vt = buffer(offset + 4, 1):uint()
	subtree:add(buffer(offset + 4, 1), string.format("VLAN Type: %i (%s)", vt, vlan_type_codes[vt] or "unk"))
end


local function parse_ports(val)
	local ports = ""
	
	for i = 8, 1, -1 do
		if (val % 2 == 1) then
			ports = ports..i.." "
		end
		val = math.floor(val / 2)
	end
	
	return ports
end


local function dissect_vlan_port_conf(buffer, offset, subtree)
	subtree:add(buffer(offset + 4, 2), string.format("VLAN: %u", buffer(offset + 4, 2):uint()))
	
	if (buffer(offset + 2, 2):uint() >= 3) then
		subtree:add(buffer(offset + 6, 1), "Ports:", parse_ports(buffer(offset + 6, 1):uint()))
	end
end


local function dissect_vlan_8021q_conf(buffer, offset, subtree)
	subtree:add(buffer(offset + 4, 2), string.format("VLAN: %u", buffer(offset + 4, 2):uint()))
	
	if (buffer(offset + 2, 2):uint() >= 4) then
		subtree:add(buffer(offset + 6, 1), "Ports:", parse_ports(buffer(offset + 6, 1):uint()))
		subtree:add(buffer(offset + 7, 1), "Tagged Ports:", parse_ports(buffer(offset + 7, 1):uint()))
	end
end


local function dissect_vlan_pvid(buffer, offset, subtree)
	subtree:add(buffer(offset + 4, 1), string.format("Port: %i", buffer(offset + 4, 1):uint()))
	subtree:add(buffer(offset + 5, 2), string.format("VLAN: %u", buffer(offset + 5, 2):uint()))
end


local function dissect_mirror(buffer, offset, subtree)
	local op = buffer(offset + 4, 1):uint()
	
	if (op == 0) then
		subtree:add(buffer(offset + 4, 1), "Disabled")
	else
		subtree:add(buffer(offset + 4, 1), "Output Port:", op)
		subtree:add(buffer(offset + 6, 1), "Ports:", parse_ports(buffer(offset + 6, 1):uint()))
	end
end


local function dissect_igmp_enablevlan(buffer, offset, subtree)
	subtree:add(buffer(offset + 4, 2), string.format("Enable: %u", buffer(offset + 4, 2):uint()))
	subtree:add(buffer(offset + 6, 2), string.format("VLAN: %u", buffer(offset + 6, 2):uint()))
end


local attributes = {
	[0x0001] = {name = "Product", dissect = "string"}, 
	[0x0003] = {name = "Name", dissect = "string"}, 
	[0x0004] = {name = "MAC", dissect = "ether"}, 
	[0x0006] = {name = "IP", dissect = "ipv4"}, 
	[0x0007] = {name = "Mask", dissect = "ipv4"}, 
	[0x0008] = {name = "Gateway", dissect = "ipv4"}, 
	[0x0009] = {name = "New Password", dissect = "string"}, 
	[0x000A] = {name = "Password", dissect = "string"}, 
	[0x000B] = {name = "DHCP", dissect = "uint"}, 
	[0x000D] = {name = "Firmware Version", dissect = "string"}, 
	[0x0010] = {name = "Firmware Upgrade", dissect = "uint"}, 
	[0x0013] = {name = "Restart", dissect = "uint"}, 
	[0x0014] = {name = "Encrypt Passwords", dissect = "uint"}, 
	[0x0400] = {name = "Defaults", dissect = "uint"}, 
	[0x0C00] = {name = "Port Status", dissect = dissect_port_status}, 
	[0x1000] = {name = "Port Statistics", dissect = dissect_port_statistics}, 
	[0x1400] = {name = "Reset Ports Statistics", dissect = "uint"}, 
	[0x1800] = {name = "Cabletest Do", dissect = nill}, 
	[0x1C00] = {name = "Cabletest Result", dissect = nill}, 
	[0x2000] = {name = "VLAN Type", dissect=dissect_vlan_type}, 
	[0x2400] = {name = "VLAN Port Conf", dissect=dissect_vlan_port_conf}, 
	[0x2800] = {name = "VLAN 802.1Q Conf", dissect=dissect_vlan_8021q_conf}, 
	[0x2C00] = {name = "Destroy VLAN", dissect = "uint"}, 
	[0x3000] = {name = "VLAN PVID", dissect = dissect_vlan_pvid}, 
	[0x3400] = {name = "QoS Type", dissect = dissect_qos_type}, 
	[0x3800] = {name = "QoS Config", dissect = dissect_qos_config}, 
	[0x4C00] = {name = "Input Bitrate", dissect = dissect_bitrate}, 
	[0x5000] = {name = "Output Bitrate", dissect = dissect_bitrate}, 
	[0x5400] = {name = "Broadcast Filtering State", dissect = "uint"}, 
	[0x5800] = {name = "Broadcast Filtering Bitrate", dissect = dissect_bitrate}, 
	[0x5C00] = {name = "Mirror", dissect = dissect_mirror}, 
	[0x6000] = {name = "Ports Count", dissect = "uint"}, 
	[0x6400] = {name = "Max 802.1Q VLAN Group", dissect = nill}, 
	[0x6800] = {name = "IGMP Enable & VLAN", dissect = dissect_igmp_enablevlan}, 
	[0x6C00] = {name = "Block Unknown IGMP Addresses", dissect = "uint"}, 
	[0x7000] = {name = "Validate IGMPv3 Headers", dissect = "uint"}, 
	[0x7400] = {name = "TLV Bitmap", dissect = nill}, 
	[0xFFFF] = {name = "End", dissect = nill}
}



local function dissect_header(buffer, subtree)
	subtree:add(f_version, buffer(0, 1))
	subtree:add(f_code, buffer(1, 1)):append_text(" ("..(op_codes[buffer(1, 1):uint()] or "unknown")..")")
	
	local errcode = buffer(2, 1):uint()
	local errattr = buffer(4, 2):uint()
	subtree:add(f_error, buffer(2, 1)):append_text(" ("..(error_codes[errcode] or "unknown")..")")
	
	-- add the erroneous attribute only if an error occurred
	if (errattr ~= 0) then
		local atf = attributes[errattr]
		subtree:add(f_errattr, buffer(4, 2)):append_text(" ("..(atf and atf.name or "unk")..")")
	end
	
	subtree:add(f_clientmac, buffer(8, 6))
	subtree:add(f_switchmac, buffer(14, 6))
	subtree:add(f_seqnum, buffer(20, 4))
end


local function dissect_attributes(buffer, subtree)
	local offset = 32
	
	while (offset < buffer:len()) do
		if (offset + 4 > buffer:len()) then
			-- no room for an attribute header, it is an error
			subtree:add(buffer(offset), "Junk"):set_expert_flags(PI_MALFORMED, PI_ERROR)
			break
		end
		
		local code = buffer(offset, 2):uint()
		local len = buffer(offset + 2, 2):uint()
		local atf = attributes[code]
		
		local attr = subtree:add(buffer(offset, math.min(4 + len, buffer:len() - offset)), string.format("Attribute: 0x%04X (%s)", code, atf and atf.name or "unk"))
		attr:add(buffer(offset, 2), string.format("Code: 0x%04X", code))
		attr:add(buffer(offset + 2, 2), string.format("Length: %u", len))
		
		if (offset + 4 + len > buffer:len()) then
			-- attribute length is bigger than remaining packet size, it is an error
			attr:append_text(" [malformed]")
			attr:set_expert_flags(PI_MALFORMED, PI_ERROR)
			break
		end
		
		
		if (len <= 0) then
			-- no data, display nothing
		elseif (atf == nil or atf.dissect == nil) then
			-- unknown attribute, display raw bytes
			attr:add(buffer(offset + 4, len), "Data:", tostring(buffer(offset + 4, len):bytes()))
		elseif (type(atf.dissect) == "function" ) then
			-- custom sub-dissector for complex type
			atf.dissect(buffer, offset, attr)
		else
			-- simple type, directly show it
			local func = assert(loadstring("return function(buffer, offset, len) return tostring(buffer(offset + 4, len):"..atf.dissect.."()) end"))() -- ugly, isn't it ?
			attr:add(buffer(offset + 4, len), atf.name..":", func(buffer, offset, len))
		end
		
		offset = offset + 4 + len
	end
end


function p_nsdp.dissector(buffer, pinfo, tree)
	pinfo.cols.protocol = p_nsdp.name
	local subtree = tree:add(p_nsdp, buffer())
	
	-- stop if the packet is too small to be valid
	if (buffer:len() < 32) then
		return
	end
	
	dissect_header(buffer, subtree)
	
	-- stop if it is just a header
	if (buffer:len() == 32) then
		return
	end
	
	local attr_list = subtree:add(buffer(32), "Attributes list")
	dissect_attributes(buffer, attr_list)
end


function p_nsdp.init ()
end


local udp_dissector_table = DissectorTable.get("udp.port")
udp_dissector_table:add(63322, p_nsdp)
udp_dissector_table:add(63321, p_nsdp)


