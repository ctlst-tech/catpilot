from fspeclib import *

Function(
    name='cube.sensors.ak09918',
    title=LocalizedString(
        en='ak09918'
    ),
    parameters=[],
    inputs=[],
    outputs=[
        Output(
            name='mx',
            title='X-axis magnetic field',
            value_type='core.type.f64'
        ),
        Output(
            name='my',
            title='Y-axis magnetic field',
            value_type='core.type.f64'
        ),
        Output(
            name='mz',
            title='Z-axis magnetic field',
            value_type='core.type.f64'
        ),
    ],
    state=[],
    parameter_constraints=[]
) 