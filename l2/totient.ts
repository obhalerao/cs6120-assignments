function main(x: bigint){
  console.log(x);
  var tot: bigint = totient(x);
  console.log(tot);
}

function totient(x: bigint): bigint{
  var ans: bigint = x;
  for(let i = 2n; i*i <= x; i++){
    if(mod(x, i) == 0n){
      for(let dummy = 0n; mod(x, i) != 0n; dummy++){
        let rem2: bigint = x/i;
        x = rem2;
      } 
      let rem3: bigint = ans/i;
      ans -= rem3;
    }
  }
  let rem1: bigint = ans/x;
  if(x > 1n) ans -= rem1;
  return ans;
}


function mod(x: bigint, y:bigint){
  var ans = x/y
  return x - y*ans;
}
