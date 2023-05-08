from fspeclib import *

Function(
    name='ctlst.io.phase_in',
    title=LocalizedString(
        en='phase_in'
    ),
    has_pre_exec_init_call=True,
    custom_cmakefile=True,
    parameters=[
        Parameter(
            name='channel',
            title='PWM channel',
            value_type='core.type.u32',
            tunable=False,
            default=0
        ),
        Parameter(
            name='total_teeth',
            title='Total teeth',
            value_type='core.type.u32',
            default=0,
        ),
        Parameter(
            name='missing_tooth',
            title='Missing teeth',
            value_type='core.type.u32',
            default=0,
        ),
        Parameter(
            name='tooth_step',
            title='Tooth time step',
            value_type='core.type.u32',
            default=0,
        ),
    ],
    inputs=[],
    outputs=[
        Output(
            name='period',
            title='Period in microseconds',
            value_type='core.type.u32'
        ),
        Output(
            name='tooth',
            title='Tooth number',
            value_type='core.type.u32'
        ),
        Output(
            name='step',
            title='Phase step',
            value_type='core.type.u32'
        )],
    state=[],
    parameter_constraints=[]
)
