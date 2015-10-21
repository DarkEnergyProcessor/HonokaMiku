--[[
Love Live! School Idol Festival game files decoder/decrypter. Part of Project HonokaMiku
For SIF JP version.

LL!SIF JP file decryption routines written in pure Lua & tested in Lua 5.1.4
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

local keyTables={
	1210253353	,1736710334	,1030507233	,1924017366	,1603299666	,1844516425	,1102797553	,32188137,
	782633907	,356258523	,957120135	,10030910	,811467044	,1226589197	,1303858438	,1423840583,
	756169139	,1304954701	,1723556931	,648430219	,1560506399	,1987934810	,305677577	,505363237,
	450129501	,1811702731	,2146795414	,842747461	,638394899	,51014537	,198914076	,120739502,
	1973027104	,586031952	,1484278592	,1560111926	,441007634	,1006001970	,2038250142	,232546121,
	827280557	,1307729428	,775964996	,483398502	,1724135019	,2125939248	,742088754	,1411519905,
	136462070	,1084053905	,2039157473	,1943671327	,650795184	,151139993	,1467120569	,1883837341,
	1249929516	,382015614	,1020618905	,1082135529	,870997426	,1221338057	,1623152467	,1020681319
}
local keyMultipler=214013
local keyAdd=2531011

md5=dofile("md5.lua")	-- source: https://github.com/kikito/md5.lua

local bxor
local bnot
do
	local _,bit=pcall(require,"bit")
	if _ then
		bxor=bit.bxor
		bnot=bit.bnot
	else
		_,bit=pcall(require,"bit32")
		if _ then
			bxor=function(a,b) return bit.bxor(a>2147483647 and a-4294967296 or a,b>2147483647 and b-4294967296 or b) end
			bnot=bit.bnot
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
			bnot=function(n)
				local p,c=1,0
				while n>0 do
					local r=n%2
					if r<1 then c=c+p end
					n,p=(n-r)/2,p*2
				end
				return c
			end
		end
	end
end

-- Returns the XOR key and the new multipler key.
-- Unlike SIF EN decrypter, both values returned as numbers instead.
local function updateKey(mkey)
	local a=(mkey*keyMultipler+keyAdd)%4294967296
	return math.floor(a/16777216),a
end

-- Decrypter setup.
-- Header is the first 16-bytes file contents
-- File is the file path in string. Actually the function just needs the basename
-- Returns the decrypter context/structure
function decryptSetup(header,file)
	local hash=md5.sum("Hello"..file:sub(-(file:reverse():find("/") or file:reverse():find("\\") or 0)+1))
	local w=hash:gmatch("....")
	local t={}
	w()	-- discard
	local hchk=w()
	hchk=bnot(hchk:sub(1,1):byte()+hchk:sub(2,2):byte()*256+hchk:sub(3,3):byte()*65536+hchk:sub(4,4):byte()*16777216)
	hchk=string.char(hchk%256)..string.char(math.floor(hchk/256)%256)..string.char(math.floor(hchk/65536)%256)
	assert(hchk==header:sub(1,3),"Header file doesn't match!")	-- Only decrypt mode 3 is supported.
	local idx=header:sub(12,12):byte()%64+1		-- +1 because Lua is 1-based indexing
	t.init_key=keyTables[idx]
	t.update_key=t.init_key
	t.xor_key=math.floor(t.init_key/16777216)
	t.pos=0
	return t
end

-- Used internally. Updates the key
local function updateKeyStruct(dctx)
	dctx.xor_key,dctx.update_key=updateKey(dctx.update_key)
end

-- Decrypt block of string(bytes).
-- dctx is decrypter context/struct
-- b is bytes that want to decrypted
-- Returns decrypted bytes
function decryptBlock(dctx,b)
	if #b==0 then return end
	local t={}
	local char=string.char
	local inst=table.insert
	for i=1,#b do
		table.insert(t,char(bxor(dctx.xor_key,b:sub(i,i):byte())))
		updateKeyStruct(dctx)
		dctx.pos=dctx.pos+1
	end
	return table.concat(t)
end

-- Sets decrypter key position to decrypt at <pos+offset> later.
-- dctx is decrypter context
-- offset is relative to current position
-- UNTESTED!
function gotoOffset(dctx,offset)
	if offset==0 then return end

	local x=dctx.pos+offset
	assert(x>=0,"Negative position")
	dctx.pos=x

	local floor=math.floor
	dctx.update_key=dctx.init_key
	dctx.xor_key=floor(key/16777216)
	if x>0 then
		for i=1,x do
			updateKeyStruct(dctx)
		end
	end
end

--[[
So, example decryption flow:

local path="file/to/tx_u_41001001_rankup_navi.texb"
local f=io.open(path,"rb")
local dctx=decryptSetup(f:read(16),path)
local f2=io.open("Honoka card #79.texb","wb")
f2:write(decryptBlock(dctx,f:read("*a")))
f2:close()
f:close()

Decrypted file is stored as "Honoka card #79.texb"
]]
