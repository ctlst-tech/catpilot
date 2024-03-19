from fspeclib import *

Function(
    name='odrive.protocol',
    title=LocalizedString(
        en='ODrive protocol atomic'
    ),
    parameters=[
        Parameter(
            name='can_if',
            title='CAN interface',
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
            name='pos',
            title='Input pos',
            value_type='core.type.f64'
        )
    ],
    outputs=[
        Output(
            name='vol',
            title='Voltage',
            value_type='core.type.f64'
        ),
        Output(
            name='cur',
            title='Current',
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
            name='main_ch',
            title='File descriptor',
            value_type='core.type.i32'
        ),
        Variable(
            name='tm_ch',
            title='File descriptor',
            value_type='core.type.i32'
        ),
    ],
)
