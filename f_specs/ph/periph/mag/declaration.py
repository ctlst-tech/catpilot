from ctlst import *

Function(
    name='ph.periph.mag',
    title=LocalizedString(
        en='MAG'
    ),
    parameters=[],
    inputs=[],
    outputs=[
        Output(
            name='magx',
            title='X-axis mag',
            value_type='core.type.f64'
        ),
        Output(
            name='magy',
            title='Y-axis mag',
            value_type='core.type.f64'
        ),
        Output(
            name='magz',
            title='Z-axis mag',
            value_type='core.type.f64'
        )
    ],
    state=[],
    parameter_constraints=[]
)
