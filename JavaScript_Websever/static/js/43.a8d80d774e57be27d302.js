webpackJsonp([43],{lmfZ:function(e,t,s){"use strict";Object.defineProperty(t,"__esModule",{value:!0});var i={render:function(){var e=this,t=e.$createElement,s=e._self._c||t;return s("el-row",{staticClass:"login-wrap"},[s("el-col",{staticClass:"col-left",attrs:{xs:12,sm:14,md:16,lg:18}},[s("h1",{staticClass:"title"},[s("span",[e._v("WiEye LIMS")]),e._v(" "),s("p",[e._v("基于无线感知步态识别的实验室管理系统")])])]),e._v(" "),s("el-col",{staticClass:"col-right",attrs:{xs:12,sm:10,md:8,lg:6}},[s("el-form",{ref:"form",staticClass:"login-form",attrs:{model:e.itemModel,size:"medium"}},[s("h2",{staticClass:"title"},[e._v("登录账号")]),e._v(" "),s("el-form-item",{attrs:{prop:"username"}},[s("el-input",{attrs:{clearable:"",autofocus:!e.itemModel.username},model:{value:e.itemModel.username,callback:function(t){e.$set(e.itemModel,"username",t)},expression:"itemModel.username"}},[s("i",{staticClass:"fas fa-user",attrs:{slot:"prefix"},slot:"prefix"})])],1),e._v(" "),s("el-form-item",{staticClass:"mar-t-md",attrs:{prop:"password"}},[s("el-input",{attrs:{type:e.isSee?"text":"password"},model:{value:e.itemModel.password,callback:function(t){e.$set(e.itemModel,"password",t)},expression:"itemModel.password"}},[s("i",{staticClass:"fas fa-key",attrs:{slot:"prefix"},slot:"prefix"}),e._v(" "),s("i",{class:"el-input__icon fas fa-"+(e.isSee?"eye":"eye-slash"),attrs:{slot:"suffix"},on:{click:function(t){e.isSee=!e.isSee}},slot:"suffix"})])],1),e._v(" "),s("el-form-item",[s("el-checkbox",{model:{value:e.itemModel.remember,callback:function(t){e.$set(e.itemModel,"remember",t)},expression:"itemModel.remember"}},[e._v("下次自动登录")])],1),e._v(" "),s("el-form-item",[s("el-button",{staticStyle:{width:"100%"},attrs:{type:"primary",loading:e.loading,disabled:e.isLoginValid},on:{click:function(t){return e.onClickLogin()}}},[e._v(e._s("登录"+(e.loading?"中":""))+"\n        ")])],1)],1)],1)],1)},staticRenderFns:[]},l=s("C7Lr")({name:"Login",data:function(){return{isSee:!1,loading:!1,itemModel:{password:"11111",username:"sinoyd"}}},computed:{isLoginValid:function(){return!this.itemModel.username||!this.itemModel.password}},methods:{onClickLogin:function(){var e=this;this.$refs.form.validate(function(t){t&&(e.loading=!0,e.$store.dispatch("auth/Login",e.itemModel).then(function(){e.$router.push("/")}).finally(function(){e.loading=!1}))})}}},i,!1,null,null,null);t.default=l.exports}});
//# sourceMappingURL=43.a8d80d774e57be27d302.js.map