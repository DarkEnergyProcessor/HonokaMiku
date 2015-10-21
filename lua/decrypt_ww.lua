--[[
Love Live! School Idol Festival game files decoder/decrypter. Part of Project HonokaMiku
For SIF EN version.

LL!SIF EN file decryption routines written in pure Lua & tested in Lua 5.1.4
Requires MD5 implementation in lua. https://github.com/kikito/md5.lua
It may not use same license below, open link above for more information.

Copyright © 2036 Dark Energy Processor Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
]]

md5=dofile("md5.lua")	-- source: https://github.com/kikito/md5.lua

local bxor
do
	local _,bit=pcall(require,"bit")
	if _ then bxor=bit.bxor
	else
		_,bit=pcall(require,"bit32")
		if _ then bxor=function(a,b) return bit.bxor(a>2147483647 and a-4294967296 or a,b>2147483647 and b-4294967296 or b) end
		else
			bxor=function(a,b)	-- source: http://stackoverflow.com/a/25594410
				local p,c=1,0
				while a>0 and b>0 do
					local ra,rb=a%2,b%2
					if ra~=rb then c=c+p end
					a,b,p=(a-ra)/2,(b-rb)/2,p*2
				end
				if a<b then a=b end
				while a>0 do
					local ra=a%2
					if ra>0 then c=c+p end
					a,p=(a-ra)/2,p*2
				end
				return c
			end
		end
	end
end

-- Returns the XOR key(2 bytes) and the new multipler key. In new tables
-- We use decimals instead of hexadecimals. I think it looks cool
local function updateKey(mkey)
	local floor=math.floor
	local _={unpack(mkey)}
	local a=mkey[1]+mkey[2]*256+mkey[3]*65536+mkey[4]*16777216
	local b=floor(a/65536)%65536
	local c=(b*1101463552)%2147483648
	local d=c+(a%65536)*16807
	local e=floor(b*16807/32768)
	local f=e+d-2147483647
	if d%4294967296>2147483646 then d=f
	else d=d+e end
	return {
		floor(d/8388608)%256,
		floor(d/32768)%256
	},
	{
		d%256,
		floor(d/256)%256,
		floor(d/65536)%256,
		floor(d/16777216)
	}
end

-- Decrypter setup.
-- Header is the first 4-bytes file contents
-- File is the file path in string. Actually the function just needs the basename
-- Returns the decrypter context/structure
function decryptSetup(header,file)
	local hash=md5.sum("BFd3EnkcKa"..file:sub(-(file:reverse():find("/") or file:reverse():find("\\") or 0)+1))
	local t={}
	local w=hash:sub(1,8):gmatch("....")
	local key=w()
	assert(header==w(),"Header file doesn't match!")	-- Currently only one decryption method is supported
	local uk={
		key:sub(4,4):byte(),
		key:sub(3,3):byte(),
		key:sub(2,2):byte(),
		key:sub(1,1):byte()%128
	}
	key=uk[1]+uk[2]*256+uk[3]*65536+uk[4]*16777216
	t.xor_key={math.floor(key/8388608)%256,math.floor(key/32768)%256}
	t.pos=0
	t.update_key=uk
	t.init_key={unpack(uk)}
	return t
end

-- Used internally. Updates the key
local function updateKeyStruct(dctx)
	local a,b=updateKey(dctx.update_key)
	dctx.update_key=b
	dctx.xor_key=a
end

-- Decrypt block of string(bytes).
-- dctx is decrypter context/struct
-- b is bytes that want to decrypted
-- Returns decrypted bytes
function decryptBlock(dctx,b)
	if #b==0 then return end
	local char=string.char
	local inst=table.insert
	local t={}
	if dctx.pos%2==1 then
		inst(t,char(bxor(b:sub(1,1):byte(),dctx.xor_key[2])))
		updateKeyStruct(dctx)
		dctx.pos=dctx.pos+1
		bytes=bytes:sub(2)
	end
	for i=1,#b do
		if i%2==1 then
			inst(t,char(bxor(b:sub(i,i):byte(),dctx.xor_key[1])))
		else
			inst(t,char(bxor(b:sub(i,i):byte(),dctx.xor_key[2])))
			updateKeyStruct(dctx)
		end
		dctx.pos=dctx.pos+1
	end
	return table.concat(t)
end

-- Sets decrypter key position to decrypt at <pos+offset> later.
-- dctx is decrypter context
-- offset is relative to current position
-- Currently slow and experimental
function gotoOffset(dctx,offset)
	if offset==0 then return end

	local x=dctx.pos+offset
	assert(x>=0,"Negative position")
	dctx.pos=x

	local floor=math.floor
	dctx.update_key={unpack(dctx.init_key)}
	local key=dctx.update_key[1]+dctx.update_key[2]*256+dctx.update_key[3]*65536+dctx.update_key[4]*16777216
	dctx.xor_key={floor(key/8388608)%256,floor(key/32768)%256}
	if x>1 then
		for i=1,floor(x/2) do
			updateKeyStruct(dctx)
		end
	end
end

--[[
So, example decryption flow:

local path="file/to/tx_u_41001001_rankup_navi.texb"
local f=io.open(path,"rb")
local dctx=decryptSetup(f:read(4),path)
local f2=io.open("Honoka card #79.texb","wb")
f2:write(decryptBlock(dctx,f:read("*a")))
f2:close()
f:close()

Decrypted file is stored as "Honoka card #79.texb"
]]
