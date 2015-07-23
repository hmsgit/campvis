-- first load all the CAMPVis Lua modules we need
require("cgt")
require("base")
require("cvio")
require("vis")
require("application")
 
-- All created CAMPVis objects have to be kept alive and must not be garbage-collected by Lua.
-- In order to keep them alive after this LuaDemo has finished, they have to reside in the global
-- Lua table. At the same time however, we do not want to spam that table. Thus, we create a
-- global table for this script, where everything resides in.
LuaDemo = {}

-- create DataContainer and Pipeline
LuaDemo.dc = application:createAndAddDataContainer("This DataContainer was created by Lua.")
LuaDemo.pipeline = campvis.AutoEvaluationPipeline(LuaDemo.dc, "A pipeline created by Lua")
local pipeline = LuaDemo.pipeline;

-- create the processors we need
local canvas_size = pipeline:getProperty("CanvasSize")
LuaDemo.lsp = base.LightSourceProvider()
LuaDemo.tcp = base.TrackballCameraProvider(canvas_size)
LuaDemo.image_reader = cvio.MhdImageReader()
LuaDemo.vr = vis.VolumeRenderer(canvas_size)

-- register the processors with the pipeline
pipeline:addProcessor(LuaDemo.lsp)
pipeline:addProcessor(LuaDemo.tcp)
pipeline:addProcessor(LuaDemo.image_reader)
pipeline:addProcessor(LuaDemo.vr)

-- setup event listener and register it with the pipeline
LuaDemo.tcp:addLqModeProcessor(LuaDemo.vr)
pipeline:addEventListenerToBack(LuaDemo.tcp)

-- create an init callback function, so that the following code gets called when the pipeline gets 
-- initialized by CAMPVis. 
-- Actually CAMPVis provides the Lua VM with an OpenGL context as well, so this code could also be 
-- here directly. However, this way we nicely can demonstrate how to use CAMPVis signals from Lua.
local initCallback = function()
    -- set up the processors' properties
    LuaDemo.vr.p_outputImage:setValue("combine")
    pipeline:getProperty("renderTargetID"):setValue("combine")

    LuaDemo.image_reader.p_url:setValue(campvis.SOURCE_DIR .. "/modules/vis/sampledata/smallHeart.mhd")
    LuaDemo.image_reader.p_targetImageID:setValue("reader.output")
    LuaDemo.image_reader.p_targetImageID:addSharedProperty(LuaDemo.vr.p_inputVolume)
    
    -- alternative 1 to automatically adjust the camera to the data
    LuaDemo.image_reader.p_targetImageID:addSharedProperty(LuaDemo.tcp.p_image)

    -- alternative 2 to automatically adjust the camera to the data
    local callback = function(arg)
        local data_container = pipeline:getDataContainer()
        local img_data = data_container:getData(LuaDemo.image_reader.p_targetImageID:getValue()):getData()
        LuaDemo.tcp:reinitializeCamera(img_data:getWorldBounds())
    end
    LuaDemo.image_reader.s_validated:connect(callback)

    -- let us create a fancy transfer function
    local dvrTF = campvis.Geometry1DTransferFunction(128, cgt.vec2(0, 0.05))
    dvrTF:addGeometry(campvis.TFGeometry1D_createQuad(cgt.vec2(0.12, 0.15), cgt.col4(85, 0, 0, 128), cgt.col4(255, 0, 0, 128)))
    dvrTF:addGeometry(campvis.TFGeometry1D_createQuad(cgt.vec2(.19, .28), cgt.col4(89, 89, 89, 155), cgt.col4(89, 89, 89, 155)))
    dvrTF:addGeometry(campvis.TFGeometry1D_createQuad(cgt.vec2(.41, .51), cgt.col4(170, 170, 128, 64), cgt.col4(192, 192, 128, 64)))

    LuaDemo.vr:getNestedProperty("RaycasterProps::TransferFunction"):replaceTF(dvrTF)
end
-- register the callback with the s_init signal
pipeline.s_init:connect(initCallback)


-- so, we now finished creating our LuaDemo. All that's missing is registering it with the CAMPVis application
application:addPipeline(pipeline)
