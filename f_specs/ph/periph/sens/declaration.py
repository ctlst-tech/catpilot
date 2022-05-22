from ctlst import *

Function(
    name='ph.periph.sens',
    title=LocalizedString(
        en='SENS'
    ),
    parameters=[],
    inputs=[],
    outputs=[
        Output(
            name='ax',
            title='X-axis acceleration',
            value_type='core.type.f32'
        ),
        Output(
            name='ay',
            title='Y-axis acceleration',
            value_type='core.type.f32'
        ),
        Output(
            name='az',
            title='Z-axis acceleration',
            value_type='core.type.f32'
        ),
        Output(
            name='wx',
            title='X-axis angular rate',
            value_type='core.type.f32'
        ),
        Output(
            name='wy',
            title='Y-axis angular rate',
            value_type='core.type.f32'
        ),
        Output(
            name='wz',
            title='Z-axis angular rate',
            value_type='core.type.f32'
        ),
    ],
    state=[],
    parameter_constraints=[]
)
