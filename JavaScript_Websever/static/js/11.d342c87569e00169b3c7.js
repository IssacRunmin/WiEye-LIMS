webpackJsonp([11],{bcf6:function(e,t,a){"use strict";Object.defineProperty(t,"__esModule",{value:!0});var i=a("4YfN"),l=a.n(i),s=a("C5/B"),n=a("rZX8"),r=a("XqJI"),o=a("rFWf"),c={name:"StaffManagement",components:{SelectTree:s.a,PagingQuery:n.a,BasicInfo:r.default,WorkExperience:o.default},data:function(){return{activeTab:"BasicInfo",loading:!1,dialogVisible:!1,id:"",itemList:[],options:[{parentId:"0",id:"A",label:"label-A",children:[{parentId:"A",id:"A-1",label:"label-A-1"}]},{parentId:"0",value:"B",label:"label-B",children:[]}],selectedIds:[],queryForm:{cName:"",userNo:"",domainId:""},pager:{page:1,rows:15,total:0}}},created:function(){this.getItemList()},methods:{closeDialog:function(){this.dialogVisible=!1,this.activeTab="BasicInfo",this.id=""},onSaveSelected:function(e){this.selectedIds=e.map(function(e){return e.id})},onQueryList:function(){this.pager.page=1,this.getItemList()},onClickAdd:function(){this.dialogVisible=!0},onClickEdit:function(e){this.id=e.id,this.dialogVisible=!0},getItemList:function(){var e=this;this.loading=!0;var t=this.pager,a=t.page,i=t.rows;this.$http.get("api/person",{params:l()({page:a,rows:i},this.queryForm)}).then(function(t){200===t.status?(e.itemList=t.data.rows,e.pager.total=t.data.total):e.$message.warning({message:t.body.message,duration:1500})},function(t){var a=t.response;e.$hp.resp(a)}).finally(function(){e.loading=!1})},onDeleteItem:function(){var e=this;0!==this.selectedIds.length?this.$confirm("此操作将永久删除该数据, 是否继续?","提示",{confirmButtonText:"确定",cancelButtonText:"取消",type:"warning"}).then(function(){return e.$http.delete("api/person/0",{data:e.selectedIds})}).then(function(t){200===t.status?(e.$message.success({message:"删除成功",duration:1500,showClose:!0}),e.getItemList()):e.$message.warning({message:t.data.message,duration:1500})},function(t){var a=t.response;e.$hp.resp(a)}):this.$message("请选择需要删除的数据")}}},u={render:function(){var e=this,t=e.$createElement,a=e._self._c||t;return a("el-card",[a("template",{slot:"header"},[a("el-form",{attrs:{inline:"",model:e.queryForm}},[a("el-form-item",{attrs:{label:"班级："}},[a("select-tree",{attrs:{options:e.options},model:{value:e.queryForm.domainId,callback:function(t){e.$set(e.queryForm,"domainId",t)},expression:"queryForm.domainId"}})],1),e._v(" "),a("el-form-item",{attrs:{label:"学号："}},[a("el-input",{model:{value:e.queryForm.userNo,callback:function(t){e.$set(e.queryForm,"userNo",t)},expression:"queryForm.userNo"}})],1),e._v(" "),a("el-form-item",{attrs:{label:"姓名："}},[a("el-input",{model:{value:e.queryForm.cName,callback:function(t){e.$set(e.queryForm,"cName",t)},expression:"queryForm.cName"}})],1),e._v(" "),a("el-form-item",[a("el-button",{attrs:{type:"primary"},on:{click:function(t){return e.onQueryList()}}},[a("i",{staticClass:"fas fa-search",attrs:{"aria-hidden":"true"}}),e._v(" 查 询\n        ")])],1)],1)],1),e._v(" "),a("el-button",{attrs:{type:"primary"},on:{click:function(t){return e.onClickAdd()}}},[a("i",{staticClass:"fas fa-plus",attrs:{"aria-hidden":"true"}}),e._v(" 新 增\n  ")]),e._v(" "),a("el-button",{attrs:{type:"danger"},on:{click:function(t){return e.onDeleteItem()}}},[a("i",{staticClass:"fas fa-trash-alt",attrs:{"aria-hidden":"true"}}),e._v(" 删 除\n  ")]),e._v(" "),a("el-table",{staticClass:"mar-t-sm",attrs:{border:"",stripe:"",full:"",data:e.itemList,loading:e.loading},on:{"selection-change":e.onSaveSelected}},[a("el-table-column",{attrs:{align:"center",type:"index",width:"50"}}),e._v(" "),a("el-table-column",{attrs:{align:"center",type:"selection"}}),e._v(" "),a("el-table-column",{attrs:{align:"center",label:"操作",width:"55"},scopedSlots:e._u([{key:"default",fn:function(t){return[a("el-button",{attrs:{type:"primary",size:"small",title:"编辑"},on:{click:function(a){return e.onClickEdit(t.row)}}},[a("i",{staticClass:"fas fa-pencil-alt",attrs:{"aria-hidden":"true"}})])]}}])}),e._v(" "),a("el-table-column",{attrs:{property:"userNO",label:"学号","show-overflow-tooltip":""}}),e._v(" "),a("el-table-column",{attrs:{property:"domainId",label:"班级","show-overflow-tooltip":""}}),e._v(" "),a("el-table-column",{attrs:{property:"cName",label:"姓名"}}),e._v(" "),a("el-table-column",{attrs:{property:"quarters",label:"职务","show-overflow-tooltip":""}}),e._v(" "),a("el-table-column",{attrs:{property:"sex",label:"性别"}})],1),e._v(" "),a("paging-query",{attrs:{pager:e.pager},on:{query:e.getItemList}}),e._v(" "),a("el-dialog",{staticClass:"pad-t-0",attrs:{fullscreen:"",title:"人员信息",visible:e.dialogVisible},on:{"update:visible":function(t){e.dialogVisible=t},close:e.closeDialog}},[[e.dialogVisible?a("el-tabs",{model:{value:e.activeTab,callback:function(t){e.activeTab=t},expression:"activeTab"}},[a("el-tab-pane",{attrs:{label:"基本信息",name:"BasicInfo"}},[a("basic-info",{on:{close:e.closeDialog,query:e.getItemList},model:{value:e.id,callback:function(t){e.id=t},expression:"id"}})],1),e._v(" "),a("el-tab-pane",{attrs:{label:"工作经历",lazy:"",name:"WorkExperience",disabled:!e.id}},[a("work-experience",{attrs:{rowId:e.id}})],1)],1):e._e()]],2)],2)},staticRenderFns:[]},d=a("C7Lr")(c,u,!1,null,null,null);t.default=d.exports}});
//# sourceMappingURL=11.d342c87569e00169b3c7.js.map