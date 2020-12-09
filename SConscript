from building import * 

# get current dir path
cwd = GetCurrentDir()

src = []
inc = [cwd]

src += ['sensor_avia_hx711.c']

if GetDepend(['PKG_USING_HX711_SAMPLE']):
    src += ['hx711_sample.c']

group = DefineGroup('hx711', src, depend = ['PKG_USING_HX711'], CPPPATH = inc)
Return('group')
