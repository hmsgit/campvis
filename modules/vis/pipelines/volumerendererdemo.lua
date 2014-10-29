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

    self.tcp:addLqModeProcessor(self.vr)
    self.addEventListenerToBack(instance, self.tcp)
end

local initCallback = function()
    pipeline.vr.p_outputImage:setValue("combine")
    pipeline.getProperty(instance, "renderTargetID"):setValue("combine")

    pipeline.image_reader.p_url:setValue(campvis.SOURCE_DIR .. "/modules/vis/sampledata/smallHeart.mhd")
    pipeline.image_reader.p_targetImageID:setValue("reader.output")
    pipeline.image_reader.p_targetImageID:addSharedProperty(pipeline.vr.p_inputVolume)
    
    -- alternative 1 to automatically adjust the camera to the data
    pipeline.image_reader.p_targetImageID:addSharedProperty(pipeline.tcp.p_image)

    -- alternative 2 to automatically adjust the camera to the data
    local callback = function(arg)
        local data_container = pipeline.getDataContainer(instance)
        local img_data = data_container:getData(pipeline.image_reader.p_targetImageID:getValue()):getData()
        pipeline.tcp:reinitializeCamera(img_data:getWorldBounds())
    end
    pipeline.image_reader.s_validated:connect(callback)

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

    pipeline.vr:getNestedProperty("RaycasterProps::TransferFunction"):replaceTF(dvrTF)
    pipeline.vr:getNestedProperty("RaycasterProps::SamplingRate"):setValue(4.0)
end

pipeline.s_init:connect(initCallback)

