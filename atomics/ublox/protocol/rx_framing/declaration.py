from fspeclib import *

var_ubx_frame = VectorTypeRef(vector_type_name='core.type.vector_u8', size=ParameterRef('ubx_frame_size'))
var_rtcm_frame = VectorTypeRef(vector_type_name='core.type.vector_u8', size=ParameterRef('rtcm_frame_size'))

Function(
    name='ublox.protocol.rx_framing',
    title="ublox GNSS protocol frame decoding",
    has_pre_exec_init_call=True,
    parameters=[
        Parameter(
            name='ubx_frame_size',
            title='UBX frame max size',
            value_type='core.type.u16',
            tunable=False,
            default=128,
        ),
        Parameter(
            name='rtcm_frame_size',
            title='UBX frame max size',
            value_type='core.type.u16',
            tunable=False,
            default=128,
        ),
        Parameter(
            name='baudrate',
            title='Baudrate',
            value_type='core.type.u32',
            tunable=False,
            default=115200,
        )
    ],
    # inputs=[
    #     Input(
    #         name='input_bool',
    #         title='input bool',
    #         value_type='core.type.bool',
    #         mandatory=False,
    #     ),
    #     Input(
    #         name='input_float',
    #         title='input float 64',
    #         value_type='core.type.f64',
    #         mandatory=False
    #     ),
    # ],
    outputs=[
        Output(
            name='ubx_frame',
            title='UBX frame',
            value_type=var_ubx_frame,
            explicit_update=True,
        ),
        Output(
            name='rtcm_frame',
            title='RTCM frame',
            value_type=var_rtcm_frame,
            explicit_update=True,
        ),
        Output(
            name='ubx_errors',
            title='Total ubx errors num',
            value_type='core.type.u32',
        ),
    ],
    state=[
        Variable(
            name='fd',
            title='File descriptor',
            value_type='core.type.i32',
        ),
        Variable(
            name='rx_buf',
            title='rx_buffer',
            value_type=VectorTypeRef(vector_type_name='core.type.vector_u8', size=256),
        ),
        Variable(
            name='rx_buf_index',
            title='Processed bytes in rx_buf',
            value_type='core.type.u32',
        ),
        Variable(
            name='rx_state',
            title='Frame parsing state',
            value_type='core.type.u32'
        ),
        Variable(
            name='rx_bytes_cnt',
            title='UBX RX bytes counter',
            value_type='core.type.u32'
        ),
        Variable(
            name='rx_unframed_bytes_cnt',
            title='Bytes within non recognized frame',
            value_type='core.type.u32'
        ),
        Variable(
            name='ubx_buf',
            title='UBX accumulation buffer',
            value_type=var_ubx_frame,
        ),
        Variable(
            name='rtcm_buf',
            title='RTCM accumulation buffer',
            value_type=var_rtcm_frame
        ),

        Variable(
            name='ubx_err_crc_cnt',
            title='UBX CRC errors counters',
            value_type='core.type.u32'
        ),
        Variable(
            name='ubx_frame_cnt',
            title='UBX Good frames counter',
            value_type='core.type.u32'
        ),

        Variable(
            name='rtcm_err_crc_cnt',
            title='RTCM CRC errors counters',
            value_type='core.type.u32'
        ),
        Variable(
            name='rtcm_frame_cnt',
            title='RTCM Good frames counter',
            value_type='core.type.u32'
        )
    ],
    parameter_constraints=[],
)
