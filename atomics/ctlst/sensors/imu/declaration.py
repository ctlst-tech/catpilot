from fspeclib import *

Function(
    name='ctlst.sensors.imu',
    title=LocalizedString(
        en='imu'
    ),
    has_pre_exec_init_call=True,
    custom_cmakefile=True,
    parameters=[
        Parameter(
            name='freq',
            title='Polling frequency',
            value_type='core.type.u32',
            tunable=False,
            default=500
        ),
        Parameter(
            name='prio_delta',
            title='Thread priority delta',
            value_type='core.type.u32',
            tunable=False,
            default=0,
        ),
    ],
    inputs=[],
    outputs=[
        Output(
            name='ax',
            title='X-axis acceleration',
            value_type='core.type.f64'
        ),
        Output(
            name='ay',
            title='Y-axis acceleration',
            value_type='core.type.f64'
        ),
        Output(
            name='az',
            title='Z-axis acceleration',
            value_type='core.type.f64'
        ),
        Output(
            name='wx',
            title='X-axis angular rate',
            value_type='core.type.f64'
        ),
        Output(
            name='wy',
            title='Y-axis angular rate',
            value_type='core.type.f64'
        ),
        Output(
            name='wz',
            title='Z-axis angular rate',
            value_type='core.type.f64'
        ),
    ],
    state=[
        Variable(
            name='period',
            title='Period us',
            value_type='core.type.u32'
        ),
    ],
    parameter_constraints=[
        ParameterValue('freq') >= 1,
        ParameterValue('freq') <= 500,
        ParameterValue('prio_delta') >= 0,
        ParameterValue('prio_delta') <= 50,
    ]
)
