-- Start
-- Script: script.lua
require("campvis")
require("vis")

print(campvis.StringProperty)
print(vis.MhdImageReader)

local nt = campvis.StringProperty("a", "b", "c", campvis.AbstractProcessor_INVALID_RESULT)
print(nt:getValue())
nt:setValue("d")
print(nt:getValue())

local mir = vis.MhdImageReader()
local iv = campvis.ivec2(50, 10)

print(vis.first(iv))
print(vis.first(campvis.ivec2_zero))

local ivp = campvis.IVec2Property("a", "b", iv, campvis.ivec2_zero, campvis.ivec2(100, 30))
local ve = vis.VolumeExplorer(ivp)
local gir = vis.GlImageResampler(ivp)

print("I am using Lua from within C++")
-- End
