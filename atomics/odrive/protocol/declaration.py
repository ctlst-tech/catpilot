from fspeclib import *

Function(
    name='odrive.protocol',
    title=LocalizedString(
        en='ODrive protocol atomic'
    ),
    parameters=[
        Parameter(
            name='can_ch',
            title='CAN cort',
            value_type='core.type.str',
            tunable=False,
        ),
        Parameter(
            name='axis',
            title='ODrive axis num',
            value_type='core.type.u32',
            tunable=False,
        ),
    ],
    description=None,
    inputs=[
        Input(
            name='input',
            title='Input',
            value_type='core.type.f64'
        )
    ],
    outputs=[
        Output(
            name='output',
            title='Output',
            value_type='core.type.f64'
        )
    ],
    state=[
        Variable(
            name='state',
            title='State',
            value_type='core.type.u32'
        ),
        Variable(
            name='fd',
            title='File Descriptor',
            value_type='core.type.i32'
        ),
    ],
)
