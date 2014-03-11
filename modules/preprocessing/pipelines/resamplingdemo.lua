require("tgt")
require("cvio")
require("vis")
require("preprocessing")

pipeline = campvis.newPipeline("ResamplingDemoLua")

function pipeline:ctor()
    print("I'm being constructed!")
    self.image_reader = cvio.MhdImageReader()
    self.addProcessor(instance, self.image_reader)

    self.ve = vis.VolumeExplorer(self.getCanvasSizeProperty(instance))
    self.resampler = preprocessing.GlImageResampler(self.getCanvasSizeProperty(instance))

    self.addProcessor(instance, self.resampler)
    self.addProcessor(instance, self.ve)

    self.addEventListenerToBack(instance, self.ve)
end

function pipeline:init()
    print("I'm being inited!")

    self.ve.p_outputImage:setValue("result")
    self.getRenderTargetIDProperty(instance):setValue("result")

    self.image_reader.p_url:setValue(campvis.SOURCE_DIR .. "/modules/vis/sampledata/smallHeart.mhd")
    self.image_reader.p_targetImageID:setValue("reader.output")
    self.image_reader.p_targetImageID:addSharedProperty(self.resampler.p_inputImage)

    self.resampler.p_outputImage:setValue("resampled")
    self.resampler.p_outputImage:addSharedProperty(self.ve.p_inputVolume)

    local geometry1 = campvis.TFGeometry1D_createQuad(tgt.vec2(0.1, 0.125), tgt.col4(255, 0, 0, 32),
                                                      tgt.vec4(255, 0, 0, 32))
    local geometry2 = campvis.TFGeometry1D_createQuad(tgt.vec2(.4, .5), tgt.col4(0, 255, 0, 128),
                                                      tgt.vec4(0, 255, 0, 128))

    local dvrTF = campvis.Geometry1DTransferFunction(128, tgt.vec2(0, 0.05))
    dvrTF:addGeometry(geometry1)
    dvrTF:addGeometry(geometry2)

    self.ve:getProperty("TransferFunction"):replaceTF(dvrTF)

    self.resampler.p_resampleScale:setValue(1.25)
end

function pipeline:deinit()
    print("I'm being deinited!")
end
