webpackJsonp([25],{"5wPU":function(t,e,a){"use strict";Object.defineProperty(e,"__esModule",{value:!0});var l={render:function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",[a("el-card",{staticClass:"title-card"},[a("div",{attrs:{slot:"header"},slot:"header"},[a("span",[t._v("采样单参数")])]),t._v(" "),a("el-row",[a("el-button",{staticClass:"mar-b-sm",attrs:{icon:"fas fa-save",type:"primary"}},[t._v(" 保 存")]),t._v(" "),a("el-button",{attrs:{icon:"fas fa-times",type:"danger"}},[t._v(" 关 闭")])],1),t._v(" "),a("el-form",{attrs:{model:t.formData,"label-width":"100px"}},[a("el-col",{attrs:{span:12}},[a("el-form-item",{staticStyle:{width:"100%"},attrs:{label:"名称",prop:"name"}},[a("el-input",{attrs:{size:"mini"},model:{value:t.formData.name,callback:function(e){t.$set(t.formData,"name",e)},expression:"formData.name"}})],1)],1),t._v(" "),a("el-col",{attrs:{span:12}},[a("el-form-item",{staticStyle:{width:"100%"},attrs:{label:"编码",prop:"code"}},[a("el-input",{attrs:{size:"mini"},model:{value:t.formData.code,callback:function(e){t.$set(t.formData,"code",e)},expression:"formData.code"}})],1)],1),t._v(" "),a("el-col",{attrs:{span:24}},[a("el-form-item",{staticStyle:{width:"100%"},attrs:{label:"对象属性",prop:"type"}},[a("el-input",{attrs:{size:"mini"},model:{value:t.formData.type,callback:function(e){t.$set(t.formData,"type",e)},expression:"formData.type"}})],1)],1)],1)],1),t._v(" "),a("el-tabs",{staticStyle:{height:"auto",overflow:"auto"},attrs:{type:"border-card"}},[a("el-tab-pane",{attrs:{label:"迁移"}},[a("el-row",{attrs:{gutter:20}},[a("el-col",{attrs:{span:12}},[a("el-button",{attrs:{type:"primary"},on:{click:t.onDebug}},[t._v("调试")]),t._v(" "),a("el-table",{ref:"debugTable",staticStyle:{width:"100%"},attrs:{data:t.tableData1}},[a("el-table-column",{attrs:{width:"50"},scopedSlots:t._u([{key:"default",fn:function(e){return[a("el-radio",{attrs:{label:e.$index},nativeOn:{change:function(a){return t.getTemplateRow(e.$index,e.row)}},model:{value:t.templateRadio,callback:function(e){t.templateRadio=e},expression:"templateRadio"}},[t._v(" ")])]}}])}),t._v(" "),t.tableColumn.hasIndex.value?a("el-table-column",{attrs:{type:"index",label:t.tableColumn.hasIndex.label,width:"50"}}):t._e(),t._v(" "),t.tableColumn.operation?a("el-table-column",{attrs:{label:t.tableColumn.operation.label},scopedSlots:t._u([{key:"default",fn:function(e){return[a("el-button",{attrs:{size:t.tableColumn.operation.size,type:t.tableColumn.operation.type,icon:t.tableColumn.operation.icon}})]}}],null,!1,3013052742)}):t._e(),t._v(" "),t._l(t.tableColumn.columnItem,function(t,e){return a("el-table-column",{key:e,attrs:{prop:t.prop,label:t.label}})})],2)],1),t._v(" "),a("el-col",{attrs:{span:12}},[a("el-table",{staticStyle:{width:"100%"},attrs:{data:t.tableData2}},[t.tableColumn.hasSelection?a("el-table-column",{attrs:{type:"selection",width:"50"}}):t._e(),t._v(" "),t.tableColumn.hasIndex.value?a("el-table-column",{attrs:{type:"index",label:t.tableColumn.hasIndex.label,width:"50"}}):t._e(),t._v(" "),t.tableColumn.operation?a("el-table-column",{attrs:{label:t.tableColumn.operation.label},scopedSlots:t._u([{key:"default",fn:function(e){return[a("el-button",{attrs:{size:t.tableColumn.operation.size,type:t.tableColumn.operation.type,icon:t.tableColumn.operation.icon}})]}}],null,!1,3013052742)}):t._e(),t._v(" "),t._l(t.tableColumn.columnItem,function(t,e){return a("el-table-column",{key:e,attrs:{prop:t.prop,label:t.label}})})],2)],1)],1)],1)],1)],1)},staticRenderFns:[]},n=a("C7Lr")({name:"SingleStepDebug",data:function(){return{formData:{name:"",code:"",type:""},tableColumn:{hasSelection:!1,hasIndex:{label:"序号",value:!0},operation:{label:"操作",content:"编 辑",size:"mini",icon:"fas fa-edit",type:"primary"},columnItem:[{label:"样品编号",prop:"itemCode"},{label:"分析项目",prop:"analyzeObj"},{label:"文本值",prop:"textValue"},{label:"名称",prop:"name"},{label:"数据值",prop:"dataValue"}]},templateRadio:0,index:0,tableData1:[],tableData2:[]}},created:function(){this.getTableData1()},methods:{getTableData1:function(){var t=this;this.$http.get("getDebugData").then(function(e){t.tableData1=e.data.tableData1})},onDebug:function(){this.index+=1,this.index<this.tableData1.length?this.templateRadio=this.index:this.$message("调试结束!")},getTemplateRow:function(t){this.index=t}}},l,!1,null,null,null);e.default=n.exports}});
//# sourceMappingURL=25.f80413097a4ac954a2e6.js.map