from fspeclib import *

Function(
    name='cube.sensors.ist8310',
    title=LocalizedString(
        en='ist8310'
    ),
    parameters=[],
    inputs=[],
    outputs=[
        Output(
            name='magx',
            title='X-axis component',
            value_type='core.type.f64'
        ),
        Output(
            name='magy',
            title='Y-axis component',
            value_type='core.type.f64'
        ),
        Output(
            name='magz',
            title='Z-axis component',
            value_type='core.type.f64'
        ),
    ],
    state=[],
    parameter_constraints=[]
)
