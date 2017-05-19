
efl = require('efl');

_print_cal_info_cb = function ()
{
   //var sel_time = cal.selected_time_get()

   var interval = cal.getInterval();
   // TODO Sort out Efl.Time support in the Js binding
   // var mm_date = cal.getDateMin();
   var mm_yr = [0, 0];
   var sel_enable = cal.getSelectMode() != efl.Elm.Calendar.Select.Mode.NONE;
   var wds = cal.getWeekdaysNames();

   console.log("weekdays= " + wds + ", interval= " + interval +
                   "\nYear_Min: "+mm_yr[0]+ ", Year_Max: "+mm_yr[1]+", Sel Enabled: "+sel_enable);
}


win = new efl.Efl.Ui.Win(null, "Efl JS Example", efl.Efl.Ui.Win.Type.BASIC, "hw");
win.setText("Calendar Getters Example");
win.setAutohide(true);

cal = new efl.Elm.Calendar(win);
cal.setHintWeight(1.0, 1.0);
win.setContent(cal);
win.setSize(240, 240);

cal.on('changed', _print_cal_info_cb);

cal.setVisible(true);
win.setVisible(true);
