webpackJsonp([32],{"1/MQ":function(e,t,l){"use strict";Object.defineProperty(t,"__esModule",{value:!0});var a={render:function(){var e=this,t=e.$createElement,l=e._self._c||t;return l("div",[l("el-form",{attrs:{inline:""}},[l("el-form-item",{attrs:{label:"样品类型："}},[l("el-select",{staticStyle:{width:"175px"},attrs:{clearable:"",filterable:""},model:{value:e.queryForm.sampleType,callback:function(t){e.$set(e.queryForm,"sampleType",t)},expression:"queryForm.sampleType"}},e._l(e.$enum.sampleTypeList,function(e,t){return l("el-option",{key:t,attrs:{label:e,value:t}})}),1)],1),e._v(" "),l("el-form-item",[l("el-button",{attrs:{type:"primary"},on:{click:function(t){return e.onQueryList()}}},[l("i",{staticClass:"fas fa-search",attrs:{"aria-hidden":"true"}}),e._v(" 查 询\n      ")])],1)],1),e._v(" "),l("hr"),e._v(" "),l("el-button",{attrs:{type:"primary"},on:{click:function(t){return e.onClickAdd()}}},[l("i",{staticClass:"fas fa-plus",attrs:{"aria-hidden":"true"}}),e._v(" 新 增\n  ")]),e._v(" "),l("el-button",{attrs:{type:"danger",disabled:!e.selectedIds.length},on:{click:function(t){return e.onDeleteItem()}}},[l("i",{staticClass:"fas fa-trash-alt",attrs:{"aria-hidden":"true"}}),e._v(" 删 除\n  ")]),e._v(" "),l("el-table",{staticClass:"mar-t-sm",attrs:{border:"",stripe:"",full:"",data:e.itemList,loading:e.loading},on:{"selection-change":e.changeSelected}},[l("el-table-column",{attrs:{align:"center",type:"selection",width:"35"}}),e._v(" "),l("el-table-column",{attrs:{align:"center",label:"序号","min-width":"45",type:"index"}}),e._v(" "),l("el-table-column",{attrs:{align:"center",label:"操作",width:"55"},scopedSlots:e._u([{key:"default",fn:function(t){return[l("el-button",{attrs:{type:"primary",size:"small",title:"编辑"},on:{click:function(l){return e.onClickEdit(t.row)}}},[l("i",{staticClass:"fas fa-pencil-alt",attrs:{"aria-hidden":"true"}})])]}}])}),e._v(" "),l("el-table-column",{attrs:{property:"kValue",label:"斜率","show-overflow-tooltip":""}}),e._v(" "),l("el-table-column",{attrs:{property:"coefficient",label:"相关系数","show-overflow-tooltip":""}}),e._v(" "),l("el-table-column",{attrs:{property:"testDate",label:"配置日期","show-overflow-tooltip":""}}),e._v(" "),l("el-table-column",{attrs:{property:"testPersonName",label:"配置人员姓名","show-overflow-tooltip":""}}),e._v(" "),l("el-table-column",{attrs:{property:"remark",label:"备注","show-overflow-tooltip":""}})],1)],1)},staticRenderFns:[]},r=l("C7Lr")({name:"",data:function(){return{loading:!1,dialogVisible:!1,itemModel:{},itemList:[],selectedIds:[],queryForm:{sampleType:"",key:""},pager:{page:1,rows:5,total:0}}},methods:{changeSelected:function(e){this.selectedIds=e.map(function(e){return e.id})},getEmptyItem:function(){return{testId:"",testName:"",testPersonName:"",testDate:"",kValue:"",bValue:"",coefficient:"",remark:"",orderNum:"",curveType:"",kRange:"",bRange:"",cRange:""}}}},a,!1,null,null,null);t.default=r.exports}});
//# sourceMappingURL=32.6e0923318803ecd3a370.js.map