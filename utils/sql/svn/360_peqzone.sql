INSERT INTO `rule_values` VALUES ('1', 'Merchant:UsePriceMod', 'true');
INSERT INTO `rule_values` VALUES ('1', 'Merchant:SellCostMod', '1.05');
INSERT INTO `rule_values` VALUES ('1', 'Merchant:BuyCostMod', '0.95');
INSERT INTO `rule_values` VALUES ('1', 'Merchant:PriceBonusPct', '4');
INSERT INTO `rule_values` VALUES ('1', 'Merchant:PricePenaltyPct', '4');
INSERT INTO `rule_values` VALUES ('1', 'Merchant:ChaBonusMod', '3.45');
INSERT INTO `rule_values` VALUES ('1', 'Merchant:ChaPenaltyMod', '1.52');

update zone set peqzone = 0 where zoneidnumber in (26 ,39 ,187 ,188 ,71 ,162 ,76 ,186 ,105 ,124 ,89 ,128 ,189 ,108 ,158 ,200 ,201); 
update zone set peqzone = 0 where zoneidnumber > 203 and zoneidnumber < 224;
update zone set peqzone = 0 where zoneidnumber > 228;