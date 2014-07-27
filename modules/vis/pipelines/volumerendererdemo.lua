require("tgt")
require("base")
require("cvio")
require("vis")
require("preprocessing")

pipeline = campvis.newPipeline("VolumeRendererDemoLua")

function pipeline:ctor()
    self.camera = campvis.CameraProperty("camera", "Camera")
    self.addProperty(instance, self.camera)

    self.lsp = base.LightSourceProvider()
    self.addProcessor(instance, self.lsp)

    self.image_reader = cvio.MhdImageReader()
    self.addProcessor(instance, self.image_reader)

    local canvas_size = self.getProperty(instance, "CanvasSize")
    self.vr = vis.VolumeRenderer(canvas_size)
    self.addProcessor(instance, self.vr)

    self.trackballEH = campvis.TrackballNavigationEventListener(self.camera, canvas_size)
    self.trackballEH:addLqModeProcessor(self.vr)

    self.addEventListenerToBack(instance, self.trackballEH)
end

function pipeline:init()
    self.camera:addSharedProperty(self.vr.p_camera)
    self.vr.p_outputImage:setValue("combine")
    self.getProperty(instance, "renderTargetID"):setValue("combine")

    self.image_reader.p_url:setValue(campvis.SOURCE_DIR .. "/modules/vis/sampledata/smallHeart.mhd")
    self.image_reader.p_targetImageID:setValue("reader.output")
    self.image_reader.p_targetImageID:addSharedProperty(self.vr.p_inputVolume)

    local callback = function(arg)
        local data_container = self.getDataContainer(instance)
        local img_data = data_container:getData(self.image_reader.p_targetImageID:getValue()):getData()
        self.trackballEH:reinitializeCamera(img_data)
    end
    self.image_reader.s_validated:connect(callback)

    local geometry1 = campvis.TFGeometry1D_createQuad(tgt.vec2(0.12, 0.15), tgt.col4(85, 0, 0, 128),
                                                      tgt.vec4(255, 0, 0, 128))
    local geometry2 = campvis.TFGeometry1D_createQuad(tgt.vec2(.19, .28), tgt.col4(89, 89, 89, 155),
                                                      tgt.vec4(89, 89, 89, 155))
    local geometry3 = campvis.TFGeometry1D_createQuad(tgt.vec2(.41, .51), tgt.col4(170, 170, 128, 64),
                                                      tgt.vec4(192, 192, 128, 64))

    local dvrTF = campvis.Geometry1DTransferFunction(128, tgt.vec2(0, 0.05))
    dvrTF:addGeometry(geometry1)
    dvrTF:addGeometry(geometry2)
    dvrTF:addGeometry(geometry3)

    self.vr:getNestedProperty("RaycasterProps::TransferFunction"):replaceTF(dvrTF)
    self.vr:getNestedProperty("RaycasterProps::SamplingRate"):setValue(4.0)
end

function pipeline:deinit()
end
