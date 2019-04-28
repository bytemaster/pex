## PEX - Pegged Token Exchange Contract

This code provides the core engine of the pegged token algorithm described in the white paper below. It does not currently compile and is provided to demonstrate the concepts described.


White Paper
-----------

# High Liquidity Price Pegged Token Algorithm
This projectintroduces a new token pegging algorithm that provides high liquidity and narrow spreads, while being robust against default in the event collateral loses value over time. The basis of our algorithm is a heavily over-collateralized short position combined with the Bancor algorithm to provide liquidity to both long and short positions. A price feed is utilized to guide the market, but its influence is limited to situations where there is a prolonged deviation so as to protect the market maker from abuse.

## Background
In 2013 BitShares introduced the concept of BitUSD, a “smart coin”, backed by BTS tokens, which was designed to track the value of the dollar. BitUSD operated by creating an order book between those who wanted leverage on the BitShares token (BTS) and those who wanted price stability. To provide liquidity for those who purchased BitUSD, BitUSD holders were allowed to force-settle the least-collateralized short position at the price feed after a multi-day delay. This created an effective margin-call and assured buyers of BitUSD that their token was always worth about a dollar worth of BTS. To prevent default, the least collateralized short position could also be force closed if the price feed fell below the minimum margin requirements.

The primary problem with the BitUSD approach is the lack of liquidity, the limited supply of BitUSD available, and the market spread. Market makers were required to operate trading bots that move orders on the internal order book. While the BitUSD holders were provided liquidity, the short positions were not guaranteed liquidity. Lastly, the entire market is subject to a black-swan event when the least collateralized position is unable to be covered. When this happens the peg is permanently broken and a fixed exchange rate is established between BitUSD and BTS.

The lack of incentive create BitUSD, the risks of low liquidity (unable to cover without unreasonable slippage), unexpected margin calls, and difficulty in running safe market making bots meant the supply of BitUSD was small and the spreads were high.

There have been many other projects that utilize variations of over-collateralized positions and margin requirements. All of them suffer similar problems to BitUSD.

Traditional derivative markets implement “futures trading” which allows people to post collateral and settle at a price feed at a fixed point in the future. These futures contracts are fungible and can be traded for a certain time, but due to the expiration and required settlement / rollover they are not suitable for creating a pegged token.

The Bancor algorithm provides automated liquidity between two assets while protecting its reserves from being lost to sophisticated traders. Regardless of the number and kind of orders executed against the Bancor algorithm, the algorithm always generates a profit when the price of the asset pair returns to its original starting point. A typical Bancor market maker is known as a relay which has two “connectors” which represent balances of equal market value. This algorithm has been successful at providing automated liquidity in markets such as the EOS RAM market.

## The Pegging Algorithm
This pegging algorithm is based upon the concept that a pegged token is a service provided by the shorts to the longs. This service requires the shorts to provide liquidity for the pegged token. If there is demand for the pegged token then trading fees earned by the market maker should be profitable for the service provider even if the market is relatively flat.

Other pegging algorithms pit shorts against each other rather than facilitate their cooperation for the purpose of bringing a pegged currency service to the market. In these other algorithms shorts compete to cover and collateralize their positions and are worried about short squeezes.

The premise of our algorithm is that those willing to be slightly leverage-long in a collateral asset (such as EOS), can make money by facilitating market making activities between EOS and a fungible pegged asset, (e.g. USD), whose value is designed to track a price feed within an allowed deviation range.

Instead of users creating independent short positions, our algorithm creates one global short position and allows users to buy and sell stake in this global position. Buying and selling stake in the short position is a neutral and reversible process (minus trading fees) provided no other trades occur in between. Instead of leverage being the primary motive, trading fees are the motive for buying stake in the position. This means that a target of 400% over collateralization or higher is viable allowing shorts to be only slightly-leveraged in the collateral asset so that they have the opportunity to earn trading fees.

The market maker is initially created by depositing collateral into the contract. The contract will create tokens in the market maker (MMS) and give them to the initial depositor. For the sake of example, we will assume the collateral is EOS and that the pegged token is USD and is designed to track the 24hr median EOS dollar price.


A target reserve ratio is decided upon, for example 400%. Based upon a 4x reserve ratio, 75% of the EOS deposited would be set aside as excess collateral, and 25% would be placed into a Bancor Relay connector with weight of 50%. At this time the automated market maker contract will create a number of USD tokens to fund a second Bancor Relay connector such that the market value of EOS and USD connector balances are the same at the initial value of the price feed.

In the initial condition the market maker owns 100% of the USD in its connector and therefore has no net liability (USD it must buy back). The book value of the MMS is equal to the value of the EOS held in the connector plus the excess collateral EOS and minus any circulating USD sold from the connector. After this initial setup anyone can buy USD from the market maker which establishes a future buy-back liability on the MMS holders. The mathematical properties of the Bancor Relay algorithm mean that as users buy USD from the market maker the offer-price will increase. If everyone who bought USD then sold it back then the connectors would end up in their initial condition. Any fees charged on the trades would result in a net increase in EOS held in the connector balance.

Given this setup and a 400% reserve target, we can demonstrate that the actual reserves vs sold USD will be far in excess of 400%. In order to get down to 400% (without market price changes) the entire USD connector would have to be bought out which would push the USD price to infinity. In effect, the market makers automatically raise the cost of buying a long USD position as the supply of available long USD positions decreases.


The spot-price offered by the market maker is equal to the ratio of EOS to USD held in the connector balances. When the spot price deviates from the feed price by more than an acceptable margin (e.g. +/- 2%) for more than an allowed period of time (e.g. 24 hours), then the market maker will do one of the following actions depending upon market conditions:

<pre>
SPOT = price of USD according to Bancor
FEED_USD = price of a federal reserve note in EOS
if( SPOT > FEED_USD ) 
   if( Excess Collateral < 3x EOS Connector )
      then slowly move EOS Connector to Excess (lowering SPOT)
   else 
     then slowly add new USD to Connector (lowering SPOT)
else if( SPOT < FEED_USD  )
   if( excess collateral )
       then gradually buy USD using excess collateral
            and destroy it (raising SPOT)
else do nothing
</pre>


The goal of this algorithm is to always move the state closer to the initial condition of having 3x more excess collateral than Bancor balance and 100% of USD in the USD connector. It is obviously impossible to achieve 100% USD in the connector while there is circulating USD, but the circulating supply can decrease in percentage terms when the value of EOS rises significantly relative to USD.

The rate at which collateral is moved or USD issued should target a price correction to the within the acceptable range within a targeted period of time (e.g. 1 hour). In theory traders should buy and sell USD as if it were worth about 1 dollar because they have confidence that they will always be able to sell it for about 1 dollar in the near future. This means that when relative price for EOS and USD is stable that the market maker will not have to rely on the price feed to correct the real time price and the security of the Bancor algorithm against manipulators protects the connector balances from being lost to traders.

When configuring parameters such as allowed deviation from feed, rate of correction, and delay until start of price correction it is critical to minimize the frequency of needing active intervention into the market maker and when intervention is needed, that intervention is slow and gradual. Market players shouldn’t be able to front-run the market maker’s moves for profit.

The result of this algorithm is a pegged asset which follows an 24-hr median value instead of an instantaneous value. It only actively corrects the peg when 24-hr median of the market maker is significantly greater than the 24-hr median of the real market. When the real market jumps we want traders to interact with the Bancor algorithm to speculatively front-run the 24-hr median. This front-running prevents the forced adjustment of the Bancor connectors from occurring except in prolonged cases. The slower the median-price moves the less risk to the market maker, but the greater deviation between pegged USD and real USD. Giving the Bancor algorithm the ability to deviate by a larger percent (say 5%) also minimizes artificial interaction with pricing due to feeds. The less artificial interaction the less relevant the instantaneous accuracy of the price feeds become.

## Buying and Selling MMS Tokens
Anyone at any time may contribute new EOS to buy a combination of MMS and USD. This works by maintaining the ratio of MMS, EOS, USD in Bancor connector and USD in circulation after adding new EOS. The individual receives MMS plus a percent of the newly created USD proportional to the USD in circulation. USD and EOS are also added to connectors and excess reserves. Users can then sell the USD for EOS and repeat the process or simply hold excess USD or EOS.

Selling MMS tokens requires providing a number of USD equal to USD_circulating * MMS_Sell / MMS_Supply. This is the same quantity of USD they would have received when buying MMS with EOS in the first place (minus any trading fees charged).

This process can be viewed as splitting and joining identical short positions. Once you control all outstanding debt of your short position you can unwind it and get all the collateral back. The key to the process is maintaining the invariant that someone buying or selling the market maker with/for collateral does not change the ratio of MMS, EOS, USD, and USD circulating. If you want to sell 1% stake in the market maker (MMS) then you must also buy and cover 1% of circulating USD. Fortunately, you can buy circulating USD from the market maker itself so there is always liquidity.

## Market Trading Fees
As users buy and sell USD, EOS, and MMS a trading fee is charged. This fee represents a revenue stream that results in capital gains to those holding MMS. The higher the volatility the more trading occurs and the more fees are generated. These fees continuously recapitalize the market maker without requiring any input from those who are collectively leveraged. Regardless of market conditions it isn’t possible for those who are leveraged to reduce their collateral or fail to gradually top it up. With a high initial collateral ratio, eg 4x, the collateral would have to fall by 75% faster than trading fees accumulate.

## Black Swans
A black swan is any event where the market maker is unable to maintain the value of USD near the price feed. This occurs when the excess collateral is gone. When this happens the market maker will continue to function but the price of USD will float independent of the feed. Astute observers will likely consider removing USD from one side of the connector to maintain the price, but this is unadvisable as it would create a run on the remaining EOS in the connector. By letting the price float once the excess collateral is gone, those who want to exit early pay a premium for the liquidity and ongoing trading will recapitalize the remaining parties.

Even during a black swan, the revenue stream represented by trading fees incentivizes parties to provide collateral and fund the market maker. In the event the collateral asset, (e.g. EOS), has no expectation of future recovery of value, then holders of the pegged asset (e.g. USD) will get a fair share of remaining EOS in the connector at market determined prices.

Unlike some other systems, a black swan event is not a special case and the market has a seamless and natural recovery method to restore the peg.

In the event excess collateral is gone, the market maker could be configured to prevent selling of MMS tokens while simultaneously allowing the buying of MMS with EOS for a 10% discount. This recapitalizes the market maker giving benefit to the new MMS holders over the prior holders. Once excess collateral is restored MMS could be sold again as described above. The exact point of excess collateral depletion and the magnitude of the discount are variables that can be adjusted to minimize the risk of complete depletion and maximize the incentive rapidly recapitalize the market without overly punishing prior holders of MMS.

Price Feeds
There are many different ways to produce a trusted price feed; however, there are some recommendations for better results. A pegged token can track any price feed, including artificial feeds such as a 30-day moving average. Typical price feeds attempt to track instantaneous spot prices, but this expectation is unrealistic for safe market makers. The slower prices change the easier it is for a peg to be maintained because market participants have more time to adjust.

It is my recommendation that pegs target the 24hr median price rather than the instantaneous price. This will reduce the frequency and magnitude of deviations from the peg without undermining the value of the pegged USD as a dollar alternative. In effect, it transfers some of the intraday volatility risk to the USD holder (deviation from the mean) while still hedging the USD holder against long-term volatility.

Some experimentation in the market will be required to determine the proper balance between reactivity of price feed and profitability of the market maker due to volatility.

## Alternative Price Correction Measures
When traders interact with the algorithm they are either pushing the bancor price further from the feed, or closer to the feed. It should be possible to have a dynamic fee on trades that grows higher the further the trade would push the bancor price from the feed. This allows MMS holders to increase profits and discourages manipulators from causing excessive deviation from the price feed.

## Conclusion
Compared to systems like BitUSD, our pegging approach incentivizes asset creation and liquidity by providing trading fees to the shorts who post collateral while effectively eliminating the majority of liquidity risks of the shorts. Furthermore, this algorithm provides equal liquidity to both sides of the market where BitUSD only provided forced-settlement to one side of the market. Trading fees continuously re-collateralize the market and enable it to recover from losses due to changing prices. So long as income from trading fees is greater than the average fall in the value of the collateral asset the system can remain solvent and liquid. We believe this approach maximizes the utility to all participants while minimizing risks.
