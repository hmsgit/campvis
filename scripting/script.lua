require("tgt")
require("cvio")
require("vis")
require("preprocessing")

pipeline = campvis.newPipeline()

function pipeline:ctor()
    print("I'm being constructed!")
    self.image_reader = cvio.MhdImageReader()
    self.addProcessor(instance, self.image_reader)

    local iv = tgt.ivec2(50, 10)

    local ivp = campvis.IVec2Property("a", "b", iv, tgt.ivec2_zero, tgt.ivec2(100, 30))
    self.ve = vis.VolumeExplorer(ivp)
    self.resampler = preprocessing.GlImageResampler(ivp)

    self.addProcessor(instance, self.resampler)
    self.addProcessor(instance, self.ve)
end

function pipeline:init()
    print("I'm being inited!")

    self.ve.p_outputImage:setValue("result")

    self.image_reader.p_url:setValue("CAMPVIS_SOURCE_DIR/modules/vis/sampledata/smallHeart.mhd")
    self.image_reader.p_targetImageID:setValue("reader.output")
    self.image_reader.p_targetImageID:addSharedProperty(self.resampler.p_inputImage)

    self.resampler.p_outputImage:setValue("resampled")
    self.resampler.p_outputImage:addSharedProperty(self.ve.p_inputVolume)

    local geometry1 = campvis.TFGeometry1D_createQuad(tgt.vec2(0.1, 0.125), tgt.col4(255, 0, 0, 32),
                                                      tgt.vec4(255, 0, 0, 32))
    local geometry2 = campvis.TFGeometry1D_createQuad(tgt.vec2(.4, .5), tgt.col4(0, 255, 0, 128),
                                                      tgt.vec4(0, 255, 0, 128))

    local dvrTF = campvis.Geometry1DTransferFunction(128, campvis.vec2(0, 0.05))
    dvrTF:addGeometry(geometry1)
    dvrTF:addGeometry(geometry2)

    self.ve:getProperty("TransferFunction"):replaceTF(dvrTF)
end

function pipeline:deinit()
    print("I'm being deinited!")
end

local nt = campvis.StringProperty("a", "b", "c", campvis.AbstractProcessor_INVALID_RESULT)
print(nt:getValue())
nt:setValue("d")
print(nt:getValue())
