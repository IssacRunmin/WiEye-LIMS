webpackJsonp([38],{"W/UR":function(e,i,a){"use strict";Object.defineProperty(i,"__esModule",{value:!0});var t=a("aA9S"),n=a.n(t),s=a("XqJI"),o=a("rFWf"),l={components:{BasicInfo:s.default,WorkExperience:o.default},data:function(){return{dialogVisible:!1,activeName:"BasicInfo",isEdit:""}},methods:{openDialog:function(e){var i=this;this.isEdit=e&&e.id,this.activeName="BasicInfo",this.$nextTick(function(){i.isEdit&&(n()(i.$refs.basicInfo.itemModel,e),i.$refs.workExperience.personId=e.id)}),this.dialogVisible=!0},closeDialog:function(){this.dialogVisible=!1,this.$refs.workExperience.personId="",this.$emit("change")},onClickTab:function(e){"WorkExperience"===e.name&&this.$refs.workExperience.getItemList()}}},c={render:function(){var e=this,i=e.$createElement,a=e._self._c||i;return a("el-dialog",{staticClass:"pad-t-0",attrs:{fullscreen:"",title:"人员信息",visible:e.dialogVisible,close:e.closeDialog},on:{"update:visible":function(i){e.dialogVisible=i}}},[e.isEdit?a("el-tabs",{on:{"tab-click":e.onClickTab},model:{value:e.activeName,callback:function(i){e.activeName=i},expression:"activeName"}},[a("el-tab-pane",{attrs:{label:"基本信息",name:"BasicInfo"}},[a("basic-info",{ref:"basicInfo",on:{close:e.closeDialog}})],1),e._v(" "),a("el-tab-pane",{attrs:{label:"工作经历",name:"WorkExperience"}},[a("work-experience",{ref:"workExperience"})],1)],1):a("el-card",[a("basic-info",{on:{close:e.closeDialog}})],1)],1)},staticRenderFns:[]},r=a("C7Lr")(l,c,!1,null,null,null);i.default=r.exports}});
//# sourceMappingURL=38.0f5592186e7f5be61544.js.map