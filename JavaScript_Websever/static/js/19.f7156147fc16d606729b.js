webpackJsonp([19],{"6ev8":function(e,r,n){"use strict";Object.defineProperty(r,"__esModule",{value:!0});var o={render:function(){var e=this,r=e.$createElement,n=e._self._c||r;return n("el-card",{staticClass:"title-card"},[n("div",{attrs:{slot:"header"},slot:"header"},[n("span",[e._v("进度条列表列表")])]),e._v(" "),n("el-table",{ref:"multipleTable",attrs:{data:e.tableData,"show-overflow-tooltip":!0,full:""}},[n("el-table-column",{attrs:{type:"index",align:"center",width:"60px"}}),e._v(" "),n("el-table-column",{attrs:{type:"selection",align:"center",width:"60px"}}),e._v(" "),n("el-table-column",{attrs:{label:"操作",align:"center",width:"95px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("el-button",{attrs:{type:"primary",title:"编辑"}},[n("i",{staticClass:"fas fa-pen"})]),e._v(" "),n("el-button",{attrs:{type:"danger",title:"删除"}},[n("i",{staticClass:"fas fa-trash-alt"})])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"企业名称",align:"center",prop:"name","show-overflow-tooltip":!0}}),e._v(" "),n("el-table-column",{attrs:{label:"企业类型",align:"center",prop:"companyType","show-overflow-tooltip":!0}}),e._v(" "),n("el-table-column",{attrs:{label:"项目所属类别",align:"center",prop:"projectType","show-overflow-tooltip":!0}}),e._v(" "),n("el-table-column",{attrs:{label:"完成进度",align:"center","show-overflow-tooltip":!0},scopedSlots:e._u([{key:"default",fn:function(r){return[n("span",{class:r.row.job},[e._v(e._s(r.row.entrust))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"任务终止",align:"center",width:"45px","show-overflow-tooltip":!0},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number4]},[e._v(e._s(r.row.number4))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"项目登记中",align:"center",width:"45px","show-overflow-tooltip":!0},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number5]},[e._v(e._s(r.row.number5))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"任务下达中",align:"center",width:"45px","show-overflow-tooltip":!0},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number6]},[e._v(e._s(r.row.number6))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"任务办理中",align:"center",prop:"number7","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number7]},[e._v(e._s(r.row.number7))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"测试中",align:"center",prop:"number8","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number8]},[e._v(e._s(r.row.number8))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"数据确认中",align:"center",prop:"number9","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number9]},[e._v(e._s(r.row.number9))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"数据审核中",align:"center",prop:"number10","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number10]},[e._v(e._s(r.row.number10))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"分析数据已办结",align:"center",prop:"number11","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number11]},[e._v(e._s(r.row.number11))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"编制报告中",align:"center",prop:"number12","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number12]},[e._v(e._s(r.row.number12))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"报告未通过",align:"center",prop:"number13","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number13]},[e._v(e._s(r.row.number13))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"报告初审中",align:"center",prop:"number14","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number14]},[e._v(e._s(r.row.number14))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"报告复审中",align:"center",prop:"number15","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number15]},[e._v(e._s(r.row.number15))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"报告审核中",align:"center",prop:"number16","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number16]},[e._v(e._s(r.row.number16))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"办结归档中",align:"center",prop:"number17","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number17]},[e._v(e._s(r.row.number17))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"已办结",align:"center",prop:"number18","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number18]},[e._v(e._s(r.row.number18))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"报告传送中",align:"center",prop:"number19","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number19]},[e._v(e._s(r.row.number19))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"报告已传送",align:"center",prop:"number20","show-overflow-tooltip":!0,width:"45px"},scopedSlots:e._u([{key:"default",fn:function(r){return[n("div",{class:e.$enum.planStatus[r.row.number20]},[e._v(e._s(r.row.number20))])]}}])}),e._v(" "),n("el-table-column",{attrs:{label:"计划投入资源(人)",align:"center",prop:"number","show-overflow-tooltip":!0}}),e._v(" "),n("el-table-column",{attrs:{label:"实际投入资源(人)",align:"center",prop:"person","show-overflow-tooltip":!0}})],1)],1)},staticRenderFns:[]};var t=n("C7Lr")({name:"ProjectPmgressbar",data:function(){return{tableData:[{name:"天堂映画",companyType:"影视文化传播",projectType:"A1",job:"text-danger",entrust:"未完成(2)",number:"12",person:"16",number4:"sure",number5:"sure",number6:"unsure",number7:"boom",number8:"boom",number9:"boom",number10:"boom",number11:"boom",number12:"boom",number13:"boom",number14:"boom",number15:"boom",number16:"boom",number17:"boom",number18:"boom",number19:"boom",number20:"boom"},{name:"三一重工",companyType:"机械制造",projectType:"S1",job:"text-success",entrust:"以完成",number:"30",person:"40",number4:"sure",number5:"sure",number6:"sure",number7:"sure",number8:"sure",number9:"sure",number10:"sure",number11:"sure",number12:"sure",number13:"sure",number14:"sure",number15:"sure",number16:"sure",number17:"sure",number18:"sure",number19:"sure",number20:"sure"},{name:"中粮集团",companyType:"农产品食品加工",projectType:"B1",job:"text-warning",entrust:"未开始",number:"10",person:"16",number4:"sure",number5:"sure",number6:"sure",number7:"sure",number8:"sure",number9:"unsure",number10:"boom",number11:"boom",number12:"boom",number13:"boom",number14:"boom",number15:"boom",number16:"boom",number17:"boom",number18:"boom",number19:"boom",number20:"boom"},{name:"华谊兄弟",companyType:"影视文化传播",projectType:"S2",job:"text-success",entrust:"已完成",number:"20",person:"26",number4:"sure",number5:"sure",number6:"sure",number7:"sure",number8:"sure",number9:"sure",number10:"sure",number11:"sure",number12:"sure",number13:"sure",number14:"sure",number15:"sure",number16:"sure",number17:"sure",number18:"sure",number19:"sure",number20:"sure"},{name:"阿里影业",companyType:"影视文化传播",projectType:"SS1",job:"text-danger",entrust:"未完成(11)",number:"60",person:"100",number4:"sure",number5:"unsure",number6:"boom",number7:"boom",number8:"boom",number9:"boom",number10:"boom",number11:"boom",number12:"boom",number13:"boom",number14:"boom",number15:"boom",number16:"boom",number17:"boom",number18:"boom",number19:"boom",number20:"boom"}]}},methods:{}},o,!1,function(e){n("mV6n")},null,null);r.default=t.exports},mV6n:function(e,r){}});
//# sourceMappingURL=19.f7156147fc16d606729b.js.map