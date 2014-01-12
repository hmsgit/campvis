require("vis")

pipeline = campvis.newPipeline()

function pipeline:ctor()
    print("I'm being constructed!")
    self.mir = vis.MhdImageReader()
    self.addProcessor(instance, self.mir)

    local iv = campvis.ivec2(50, 10)

    print(vis.first(iv))
    print(vis.first(campvis.ivec2_zero))

    local ivp = campvis.IVec2Property("a", "b", iv, campvis.ivec2_zero, campvis.ivec2(100, 30))
    self.ve = vis.VolumeExplorer(ivp)
    self.gir = vis.GlImageResampler(ivp)

    --self.addProcessor(instance, self.gir)
    --self.addProcessor(instance, self.ve)
end

function pipeline:init()
    print("I'm being inited!")
end

function pipeline:deinit()
    print("I'm being deinited!")
end

local nt = campvis.StringProperty("a", "b", "c", campvis.AbstractProcessor_INVALID_RESULT)
print(nt:getValue())
nt:setValue("d")
print(nt:getValue())
