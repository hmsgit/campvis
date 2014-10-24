require("cgt")
require("base")
require("cvio")
require("vis")
require("preprocessing")

pipeline = campvis.newPipeline("VolumeRendererDemoLua")

function pipeline:ctor()
    local canvas_size = self.getProperty(instance, "CanvasSize")

    self.lsp = base.LightSourceProvider()
    self.addProcessor(instance, self.lsp)

    self.tcp = base.TrackballCameraProvider(canvas_size)
    self.addProcessor(instance, self.tcp)

    self.image_reader = cvio.MhdImageReader()
    self.addProcessor(instance, self.image_reader)

    self.vr = vis.VolumeRenderer(canvas_size)
    self.addProcessor(instance, self.vr)

    -- alternative 1 to automatically adjust the camera to the data
    self.tcp:addLqModeProcessor(self.vr)

    self.addEventListenerToBack(instance, self.tcp)
end

function pipeline:init()
    self.vr.p_outputImage:setValue("combine")
    self.getProperty(instance, "renderTargetID"):setValue("combine")

    self.image_reader.p_url:setValue(campvis.SOURCE_DIR .. "/modules/vis/sampledata/smallHeart.mhd")
    self.image_reader.p_targetImageID:setValue("reader.output")
    self.image_reader.p_targetImageID:addSharedProperty(self.vr.p_inputVolume)
    self.image_reader.p_targetImageID:addSharedProperty(self.tcp.p_image)

    -- alternative 2 to automatically adjust the camera to the data
    local callback = function(arg)
        local data_container = self.getDataContainer(instance)
        local img_data = data_container:getData(self.image_reader.p_targetImageID:getValue()):getData()
        self.tcp:reinitializeCamera(img_data:getWorldBounds())
    end
    self.image_reader.s_validated:connect(callback)

    local geometry1 = campvis.TFGeometry1D_createQuad(cgt.vec2(0.12, 0.15), cgt.col4(85, 0, 0, 128),
                                                      cgt.vec4(255, 0, 0, 128))
    local geometry2 = campvis.TFGeometry1D_createQuad(cgt.vec2(.19, .28), cgt.col4(89, 89, 89, 155),
                                                      cgt.vec4(89, 89, 89, 155))
    local geometry3 = campvis.TFGeometry1D_createQuad(cgt.vec2(.41, .51), cgt.col4(170, 170, 128, 64),
                                                      cgt.vec4(192, 192, 128, 64))

    local dvrTF = campvis.Geometry1DTransferFunction(128, cgt.vec2(0, 0.05))
    dvrTF:addGeometry(geometry1)
    dvrTF:addGeometry(geometry2)
    dvrTF:addGeometry(geometry3)

    self.vr:getNestedProperty("RaycasterProps::TransferFunction"):replaceTF(dvrTF)
    self.vr:getNestedProperty("RaycasterProps::SamplingRate"):setValue(4.0)
end

function pipeline:deinit()
end
