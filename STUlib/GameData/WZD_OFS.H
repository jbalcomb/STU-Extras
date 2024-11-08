#ifndef WZD_OFS_H
#define WZD_OFS_H


/*

RT     Race Table
UTN    Unit Type Name
UTT    Unit Type Table


HoF Race Names

dseg:59B4
2EE54
3C454

dseg:59D0
2EE70
3C470

dseg:5A4D
2EEED
3C4ED



Unit Type Race Name
race_names[rvr_unit_type->Race], (char*)(wzd_buf + WZD_OFS_DSEG + unit_type_name_data_segment_offset));

race_table
s_UNIT.type > s_UNITTYPE.race > unit_race_table  s_RACE.name

*/


/* Offset to Data Segment */
#define WZD_OFS_DSEG                0x000294A0


/* dseg:019C  2963C  36C3C */
#define WZD_OFS_UTT                 0x0002963C
#define WZD_OFS_UNIT_TYPE_TABLE     0x0002963C

/* dseg:1D86  2B226  38826 */
#define WZD_OFS_RT                  0x0002B226
#define WZD_OFS_RACE_TABLE          0x0002B226


/* dseg:21D3  2B673  38C73 */
#define WZD_OFS_UTN                 0x0002B673
#define WZD_OFS_UNIT_TYPE_NAME      0x0002B673



/* dseg:019C  2963C  36C3C */
#define WZD_DSEG_OFS_UTT            0x0002963C

/* dseg:1D86  2B226  38826 */
#define WZD_DSEG_OFS_RT             0x1D86

/* Offset to Unit Type Names */
#define WZD_DSEG_OFS_UTN            0x21D3



#endif  /* WZD_OFS_H */
