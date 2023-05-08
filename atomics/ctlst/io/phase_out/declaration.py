from fspeclib import *

Function(
    name='ctlst.io.phase_out',
    title=LocalizedString(
        en='phase_out'
    ),
    has_pre_exec_init_call=True,
    custom_cmakefile=True,
    parameters=[
        Parameter(
            name='channel',
            title='Phase channel',
            value_type='core.type.u32',
            tunable=False,
            default=0
        ),
        Parameter(
            name='invert',
            title='Output invert',
            value_type='core.type.u32',
            default=0,
        ),
        Parameter(
            name='time_mode',
            title='Time/phase mode',
            value_type='core.type.u32',
            default=0,
        ),
        Parameter(
            name='sync_channel',
            title='Channel to sync',
            value_type='core.type.u32',
            default=0,
        ),
        Parameter(
            name='tooth',
            title='Tooth',
            value_type='core.type.u32',
            default=0,
        ),
        Parameter(
            name='phase_on',
            title='Phase on',
            value_type='core.type.u32',
            default=0,
        ),
        Parameter(
            name='phase_off',
            title='Phase off',
            value_type='core.type.u32',
            default=0,
        ),
    ],
    inputs=[],
    outputs=[],
    state=[],
    parameter_constraints=[]
)
