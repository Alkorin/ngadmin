
p_nsdp=Proto("nsdp", "Netgear Switch Description Protocol")
local f_version=ProtoField.uint8("nsdp.version", "Version", base.DEC)
local f_code=ProtoField.uint8("nsdp.code", "Operation Code", base.DEC)
local f_error=ProtoField.uint8("nsdp.error", "Error Code", base.DEC)
local f_errattr=ProtoField.uint16("nsdp.errattr", "Erroneous Attribute", base.HEX)
local f_clientmac=ProtoField.ether("nsdp.clientmac", "Client MAC")
local f_switchmac=ProtoField.ether("nsdp.switchmac", "Switch MAC")
local f_seqnum=ProtoField.uint32("nsdp.seqnum", "Sequence Number", base.DEC)

local f_attr=ProtoField.uint16("", "Attribute", base.HEX)
local f_attr_code=ProtoField.uint16("", "Attribute Code", base.HEX)
local f_attr_length=ProtoField.uint16("", "Attribute Length", base.DEC)
local f_attr_data=ProtoField.bytes("", "Attribute Data")

local f_attr_product=ProtoField.string("", "Product")
local f_attr_name=ProtoField.string("", "Name")
local f_attr_mac=ProtoField.ether("", "MAC")
local f_attr_ip=ProtoField.ipv4("", "IP")
local f_attr_mask=ProtoField.ipv4("", "Mask")
local f_attr_gateway=ProtoField.ipv4("", "Gateway")
local f_attr_newpassword=ProtoField.string("", "New Password")
local f_attr_password=ProtoField.string("", "Password")
local f_attr_dhcp=ProtoField.bool("", "DHCP")
local f_attr_firmver=ProtoField.string("", "Firmare Version")
local f_attr_portscount=ProtoField.uint8("", "Ports Count", base.DEC)
local f_attr_port=ProtoField.uint8("", "Port", base.DEC)
local f_attr_port_status=ProtoField.uint8("", "Port Status", base.DEC)

p_nsdp.fields={
 f_version, f_code, f_error, f_errattr, f_clientmac, f_switchmac, f_seqnum, 
 f_attr, f_attr_code, f_attr_length, f_attr_data, 
 f_attr_product, f_attr_name, f_attr_mac, f_attr_ip, f_attr_mask, f_attr_gateway, f_attr_newpassword, f_attr_password, f_attr_portscount, 
 f_attr_dhcp, f_attr_port, f_attr_port_status, f_attr_firmver
}



local op_codes={
 [1]="Read Request", 
 [2]="Read Reply", 
 [3]="Write Request", 
 [4]="Write Reply"
}


local error_codes={
 [0]="OK", 
 [5]="Invalid Value", 
 [7]="Invalid Password"
}


local status_codes={
 [0]="down", 
 [4]="100M", 
 [5]="1000M"
}





function dissect_port_status (buffer, offset, subtree)
 
 subtree:add(f_attr_port, buffer(offset+4, 1))
 subtree:add(f_attr_port_status, buffer(offset+5, 1)):append_text(" ("..(status_codes[buffer(offset+5, 1):uint()] or "unk")..")")
 
end





local attributes={
 [0x0001]={name="Product", dissect=f_attr_product}, 
 [0x0003]={name="Name", dissect=f_attr_name}, 
 [0x0004]={name="MAC", dissect=f_attr_mac}, 
 [0x0006]={name="IP", dissect=f_attr_ip}, 
 [0x0007]={name="Mask", dissect=f_attr_mask}, 
 [0x0008]={name="Gateway", dissect=f_attr_gateway}, 
 [0x0009]={name="New Password", dissect=f_attr_newpassword}, 
 [0x000A]={name="Password", dissect=f_attr_password}, 
 [0x000B]={name="DHCP", dissect=f_attr_dhcp}, 
 [0x000D]={name="Firmware Version", dissect=f_attr_firmver}, 
 [0x0C00]={name="Port Status", dissect=dissect_port_status}, 
 [0x6000]={name="Ports Count", dissect=f_attr_portscount}
}





function dissect_header (buffer, subtree)
 
 subtree:add(f_version, buffer(0, 1))
 
 subtree:add(f_code, buffer(1, 1)):append_text(" ("..(op_codes[buffer(1, 1):uint()] or "unknown")..")")
 
 local errcode=buffer(2, 1):uint()
 subtree:add(f_error, buffer(2, 1)):append_text(" ("..(error_codes[errcode] or "unknown")..")")
 
 -- print the erroneous attribute if an error occurred
 if ( errcode~=0 ) then
  local atf=attributes[buffer(4, 2):uint()]
  subtree:add(f_errattr, buffer(4, 2)):append_text(" ("..(atf and atf.name or "unk")..")")
 end
 
 subtree:add(f_clientmac, buffer(8, 6))
 
 subtree:add(f_switchmac, buffer(14, 6))
 
 subtree:add(f_seqnum, buffer(20, 4))
 
 
end



function dissect_attributes (buffer, subtree)
 
 local offset=32
 
 while ( offset<buffer:len() ) do
  
  local code=buffer(offset, 2):uint()
  local len=buffer(offset+2, 2):uint()
  local atf=attributes[code]
  
  local attr=subtree:add(f_attr, buffer(offset, 4+len), code)
  attr:append_text(" ("..(atf and atf.name or "unk")..")")
  
  attr:add(f_attr_code, buffer(offset, 2))
  
  attr:add(f_attr_length, buffer(offset+2, 2))
  
  if ( len<=0 ) then
   -- no data, display nothing
  elseif ( atf==nil ) then
   -- unknown attribute, display raw bytes
   attr:add(f_attr_data, buffer(offset+4, len))
  elseif ( type(atf.dissect)=="function" ) then
   -- custom sub-dissector for complex type
   atf.dissect(buffer, offset, attr)
  else
   -- simple type, directly use field
   attr:add(atf.dissect, buffer(offset+4, len))
  end
  
  offset=offset+4+len
  
 end
 
end



function p_nsdp.dissector (buffer, pinfo, tree)
 
 pinfo.cols.protocol=p_nsdp.name
 local subtree=tree:add(p_nsdp, buffer())
 
 -- stop if the packet is too small to be valid
 if ( buffer:len()<32 ) then return end
 
 dissect_header(buffer, subtree)
 
 -- stop if it is just a header
 if ( buffer:len()<=32 ) then return end
 
 local attr_list=subtree:add(buffer(32), "Attributes list")
 dissect_attributes(buffer, attr_list)
 
end



function p_nsdp.init ()
end



local udp_dissector_table=DissectorTable.get("udp.port")
dissector=udp_dissector_table:get_dissector(63322)
udp_dissector_table:add(63322, p_nsdp)


